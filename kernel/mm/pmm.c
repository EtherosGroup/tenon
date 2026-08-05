#include "pmm.h"
#include "memlayout.h"
#include "multiboot.h"
#include "serial.h"
#include "asm.h"

/**
 * 状态
 */
static u64 total_pages; // 总页数
static u64 free_count; // 总空闲页数
static u8 *bitmap; // 位图指针，指向物理地址 bitmap_phys，每个 bit 代表一个页框
static u64 bitmap_phys; // 位图占用区域的物理起始地址
static u64 bitmap_pages; // 位图占用的 4 KiB 页数

// L1 分配缓存：存放最近释放的页号，分配时栈顶弹出 O(1) 命中
#define FREE_STACK_SIZE 512
static u64 free_stack[FREE_STACK_SIZE];
static u32 stack_top;

static memory_region_type memory_regions[MAX_MEMORY_REGIONS];
static u32 memory_region_count;

static bool have_fb_tag;
static multiboot_tag_fb_type saved_fb_tag;

void pmm_get_fb_tag(multiboot_tag_fb_type *out, bool *valid)
{
    *valid = have_fb_tag;
    if (have_fb_tag)
    {
        *out = saved_fb_tag;
    }
}

/**
 * 将第 page 号页框标记为已用（bit 置 1）
 * page / 8 定位到字节，page % 8 定位到 bit
 */
static void bitmap_set(u64 page)
{
    bitmap[page / 8] |= (u8)(1 << (page % 8));
}

/**
 * 将第 page 号页框标记为空闲（bit 清 0）
 */
static void bitmap_clear(u64 page)
{
    bitmap[page / 8] &= (u8)~(1 << (page % 8));
}

/**
 * 查询第 page 号页框是否已用，返回 false（空闲）或 true（已用）
 */
static bool bitmap_test(u64 page)
{
    return (bitmap[page / 8] >> (page % 8)) & 1;
}

/**
 * 将页号推入释放缓存栈
 * 栈满时静默丢弃，被丢弃的页下次分配时通过位图线性扫描找回
 */
static void stack_push(u64 page)
{
    if (stack_top < FREE_STACK_SIZE)
    {
        free_stack[stack_top++] = page;
    }
}

/**
 * 从释放缓存栈弹出一个页号
 * 栈空时返回 (u64)-1，调用方自行判断并 fallback 到位图扫描
 */
static u64 stack_pop(void)
{
    if (stack_top > 0)
    {
        return free_stack[--stack_top];
    }
    return (u64) - 1;
}

/**
 * 将 [base, base+length) 物理地址范围标记为"已用" （左闭右开666死去的数学开始攻击我）
 * 对齐策略：PAGE_ALIGN_DOWN(base) 到 PAGE_ALIGN_UP(base+length)，
 * 确保区域两端的不完整页也被覆盖
 */
static void bitmap_mark_region_used(u64 base, u64 length)
{
    u64 start = ADDR_TO_PAGE(PAGE_ALIGN_DOWN(base));
    u64 end = ADDR_TO_PAGE(PAGE_ALIGN_UP(base + length));
    for (u64 p = start; p < end && p < total_pages; p++)
    {
        if (!bitmap_test(p))
        {
            bitmap_set(p);
            free_count--;
        }
    }
}

/*
 * 将 [base, base+length) 物理地址范围标记为"空闲"
 */
static void bitmap_mark_region_free(u64 base, u64 length)
{
    u64 start = ADDR_TO_PAGE(PAGE_ALIGN_DOWN(base));
    u64 end = ADDR_TO_PAGE(PAGE_ALIGN_UP(base + length));
    for (u64 p = start; p < end && p < total_pages; p++)
    {
        if (bitmap_test(p))
        {
            bitmap_clear(p);
            free_count++;
        }
    }
}

void pmm_init(u32 magic, u64 info_ptr)
{
    u64 max_phys = 0;
    memory_region_count = 0;

    if (magic != MULTIBOOT2_MAGIC)
    {
        serial_print("[PMM] No Multiboot2 (magic=");
        serial_print_hex(magic);
        serial_println("), fallback: 128 MiB");

        max_phys = 128ULL * 1024 * 1024;
        memory_regions[0].base = 0;
        memory_regions[0].length = max_phys;
        memory_region_count = 1;
    }
    else
    {
        multiboot_info_type *info = (multiboot_info_type *)info_ptr;
        u8 *tag_start = (u8 *)info_ptr + 8;
        u8 *tag_end = (u8 *)info_ptr + info->total_size;

        for (u8 *p = tag_start; p < tag_end; )
        {
            multiboot_tag_type *tag = (multiboot_tag_type *)p;
            if (tag->type == MULTIBOOT_TAG_END)
            {
                break;
            }

            if (tag->type == MULTIBOOT_TAG_MMAP)
            {
                multiboot_tag_mmap_type *mmap = (multiboot_tag_mmap_type *)p;
                u32 entry_count = (mmap->size - sizeof(multiboot_tag_mmap_type)) / mmap->entry_size;
                multiboot_mmap_entry_type *entry = (multiboot_mmap_entry_type *)(p + sizeof(multiboot_tag_mmap_type));

                for (u32 i = 0; i < entry_count; i++)
                {
                    if (entry[i].type == MULTIBOOT_MEMORY_AVAILABLE)
                    {
                        u64 end = entry[i].base_addr + entry[i].length;
                        if (end > max_phys)
                        {
                            max_phys = end;
                        }

                        if (memory_region_count < MAX_MEMORY_REGIONS)
                        {
                            memory_regions[memory_region_count].base = entry[i].base_addr;
                            memory_regions[memory_region_count].length = entry[i].length;
                            memory_region_count++;
                        }
                    }
                }
            }

            if (tag->type == MULTIBOOT_TAG_FRAMEBUFFER)
            {
                multiboot_tag_fb_type *fb = (multiboot_tag_fb_type *)p;
                saved_fb_tag = *fb;
                saved_fb_tag.type = fb->type;
                saved_fb_tag.size = fb->size;
                have_fb_tag = true;
            }

            p += (tag->size + 7) & ~7U;
        }
    }

    // 计算总页数和 bitmap 大小
    total_pages = PAGE_ALIGN_UP(max_phys) / PAGE_SIZE;
    bitmap_pages = PAGE_ALIGN_UP(total_pages / 8) / PAGE_SIZE;
    free_count = 0;

    // 扫描 memory_regions 找首个足够大的区域，跳过内核占用，限制 1 GiB 内
    bitmap_phys = 0;
    for (u32 i = 0; i < memory_region_count; i++)
    {
        u64 candidate = memory_regions[i].base;
        u64 region_end = candidate + memory_regions[i].length;

        if (candidate < (u64)__kernel_phys_end)
        {
            candidate = PAGE_ALIGN_UP((u64)__kernel_phys_end);
        }

        if (candidate >= 0x40000000ULL)
        {
            continue;
        }

        if (candidate + bitmap_pages * PAGE_SIZE <= region_end)
        {
            bitmap_phys = candidate;
            break;
        }
    }
    if (!bitmap_phys)
    {
        serial_println("[PMM] No region large enough for bitmap, halting.");
        for (;;)
        {
            asm_hlt();
        }
    }

    // 初始化 bitmap 全为 1（全部标记已用）
    bitmap = (u8 *)bitmap_phys;
    for (u64 i = 0; i < bitmap_pages * PAGE_SIZE; i++)
    {
        bitmap[i] = 0xFF;
    }
    free_count = 0;

    // 第 2 遍：将可用区域标记为空闲
    for (u32 i = 0; i < memory_region_count; i++)
    {
        bitmap_mark_region_free(memory_regions[i].base, memory_regions[i].length);
    }

    // 标记 bitmap 自身占用
    bitmap_mark_region_used(bitmap_phys, bitmap_pages * PAGE_SIZE);

    /**
     * 标记内核占用
     * 1 MiB ~ bss 尾部
     */
    u64 kernel_start = (u64)__kernel_phys_start;
    u64 kernel_end = (u64)__kernel_phys_end;
    bitmap_mark_region_used(kernel_start, kernel_end - kernel_start);

    /**
     * 标记前 1 MiB 为已用
     * BIOS/IVT/EBDA 等
     */
    bitmap_mark_region_used(0, 0x100000);

    // 初始化 free_stack
    stack_top = 0;

    serial_println("[PMM] Initialized.");
}


u64 pmm_alloc_page(void)
{
    u64 page = stack_pop();
    if (page != (u64) - 1)
    {
        bitmap_set(page);
        free_count--;
        return PAGE_TO_ADDR(page);
    }

    for (u64 p = 0; p < total_pages; p++)
    {
        if (!bitmap_test(p))
        {
            bitmap_set(p);
            free_count--;
            return PAGE_TO_ADDR(p);
        }
    }

    return 0;
}

u64 pmm_alloc_page_below(u64 max_phys)
{
    for (u64 p = 0; p < total_pages && PAGE_TO_ADDR(p) < max_phys; p++)
    {
        if (!bitmap_test(p))
        {
            bitmap_set(p);
            free_count--;
            return PAGE_TO_ADDR(p);
        }
    }
    return 0;
}

void pmm_free_page(u64 phys_addr)
{
    u64 page = ADDR_TO_PAGE(phys_addr);
    if (page >= total_pages)
    {
        return;
    }
    if (!bitmap_test(page))
    {
        return;
    }

    bitmap_clear(page);
    free_count++;
    stack_push(page);
}

u64 pmm_total_pages(void)
{
    return total_pages;
}

u64 pmm_free_pages(void)
{
    return free_count;
}

u32 pmm_get_memory_regions(memory_region_type *regions, u32 max_count)
{
    u32 count = memory_region_count < max_count ? memory_region_count : max_count;
    for (u32 i = 0; i < count; i++)
    {
        regions[i] = memory_regions[i];
    }
    return count;
}