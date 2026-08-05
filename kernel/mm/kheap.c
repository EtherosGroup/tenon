#include "kheap.h"
#include "memlayout.h"
#include "pmm.h"
#include "vmm.h"
#include "serial.h"

typedef struct heap_block
{
    u64 size;
    struct heap_block *next;
} heap_block_t;

static heap_block_t *free_list;
static u64 heap_top;
static u64 heap_end;

static u64 align_up(u64 size, u64 align)
{
    return (size + align - 1) & ~(align - 1);
}

static bool grow_heap(u64 min_size)
{
    u64 needed = align_up(min_size, PAGE_SIZE);
    u64 pages = needed / PAGE_SIZE;

    if (heap_top + needed > heap_end)
    {
        serial_println("[KHEAP] Out of virtual address space.");
        return false;
    }

    for (u64 i = 0; i < pages; i++)
    {
        u64 phys = pmm_alloc_page();
        if (!phys)
        {
            serial_println("[KHEAP] Out of physical memory.");
            return true;
        }
        vmm_map_page(&kernel_as, heap_top + i * PAGE_SIZE, phys, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
    }

    heap_block_t *block = (heap_block_t *)heap_top;
    block->size = pages * PAGE_SIZE;
    block->next = free_list;
    free_list = block;

    heap_top += pages * PAGE_SIZE;
    return true;
}

void kheap_init(void)
{
    free_list = null;
    heap_top = KERNEL_HEAP_START;
    heap_end = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;
    serial_println("[KHEAP] Initialized.");
}

void *kmalloc(u64 size)
{
    if (!size)
    {
        return null;
    }

    u64 total_size = align_up(size + sizeof(heap_block_t), KHEAP_MIN_ALLOC);

    // 遍历 free_list, first-fit
    heap_block_t **prev = &free_list;
    for (heap_block_t *b = free_list; b; prev = &b->next, b = b->next)
    {
        if (b->size >= total_size)
        {
            // 能切分就切分
            if (b->size >= total_size + sizeof(heap_block_t) + KHEAP_MIN_ALLOC)
            {
                heap_block_t *new_block = (heap_block_t *)((u8 *)b + total_size);
                new_block->size = b->size - total_size;
                new_block->next = b->next;
                *prev = new_block;
                b->size = total_size;
            }
            else
            {
                *prev = b->next;
            }
            return (u8 *)b + sizeof(heap_block_t);
        }
    }

    // 没有合适的空闲块, 扩展堆
    if (!grow_heap(total_size))
    {
        return null;
    }
    return kmalloc(size); // 重试
}

void kfree(void *ptr)
{
    if (!ptr)
    {
        return;
    }

    heap_block_t *block = (heap_block_t *)((u8 *)ptr - sizeof(heap_block_t));

    // 插入到 free_list 头部（简单, 不做合并）
    block->next = free_list;
    free_list = block;
}

void *krealloc(void *ptr, u64 new_size)
{
    if (!ptr)
    {
        return kmalloc(new_size);
    }
    if (!new_size)
    {
        kfree(ptr);
        return null;
    }

    heap_block_t *block = (heap_block_t *)((u8 *)ptr - sizeof(heap_block_t));
    u64 old_payload = block->size - sizeof(heap_block_t);

    if (old_payload >= new_size)
    {
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);
    if (!new_ptr)
    {
        return null;
    }

    for (u64 i = 0; i < old_payload; i++)
    {
        ((u8 *)new_ptr)[i] = ((u8 *)ptr)[i];
    }

    kfree(ptr);
    return new_ptr;
}