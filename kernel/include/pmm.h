#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include "types.h"

#define MAX_MEMORY_REGIONS 32

/**
 * 描述一段连续的可用物理内存区域
 * @field base 区域起始物理地址
 * @field length 区域长度（字节），不一定页对齐
 */
typedef struct
{
    u64 base;
    u64 length;
} memory_region_type;

/**
 * 初始化物理内存管理器（必须最先调用）
 *
 * 从 Multiboot2 内存布局 tag 解析可用物理内存区域，建立位图
 * 若 magic 不等于 MULTIBOOT2_MAGIC（非 Multiboot2 启动），自动 fallback 为 128 MiB 物理内存
 *
 * @param magic GRUB/Multiboot2 魔数，EAX 传入
 * @param info_ptr Multiboot2 info 结构体物理地址，EBX 传入
 */
void pmm_init(u32 magic, u64 info_ptr);

/**
 * 分配一个 4 KiB 物理页框
 *
 * 策略：优先从 free_stack（最近释放的页缓存）弹出一个 O(1) 分配；
 * 栈空时线性扫描位图，返回首个空闲页 O(n)。
 * 分配后位图对应 bit 置 1，free_count--。
 *
 * @return 页对齐的物理地址，无可用页时返回 0
 */
u64 pmm_alloc_page(void);

/**
 * 在指定物理地址上限以下分配一个 4 KiB 物理页框
 *
 * 纯线性扫描位图，不走 free_stack（栈中页可能来自任意地址，
 * 不满足上限约束）。直接扫描到首个满足 page_addr <= max_phys
 * 的空闲页。
 *
 * 典型场景：vmm_init 中分配页表页，在 CR3 切换前必须约束于
 * 1 GiB 恒等映射范围内。
 *
 * @param max_phys 物理地址上限（含），分配返回的地址 <= max_phys
 * @return 页对齐的物理地址，无可用页时返回 0
 */
u64 pmm_alloc_page_below(u64 max_phys);

/**
 * 释放一个物理页框
 *
 * 位图对应 bit 清 0，free_count++，并将页号推入 free_stack。
 * 若 phys_addr 非页对齐、越界，或该页已空闲（二次释放），
 * 直接返回不做任何操作，保证 free_count 不被破坏。
 *
 * @param phys_addr 由 pmm_alloc_page 返回的精确物理地址
 */
void pmm_free_page(u64 phys_addr);

/**
 * 查询物理内存管理器可管理的物理页总数（含已用和空闲）
 * @return 总页框数（total_pages = max_phys / 4096 向上取整）
 */
u64 pmm_total_pages(void);

/**
 * 查询当前空闲物理页数量
 * @return 空闲页框数
 */
u64 pmm_free_pages(void);

/**
 * 获取可用物理内存区域列表（拷贝输出）
 *
 * 将 pmm_init 中解析出的内存区域拷贝到调用方提供的数组中
 * vmm 用于建立直接物理映射窗口
 *
 * @param regions 输出缓冲区
 * @param max_count 缓冲区最大条目数
 * @return 实际写入的条目数（不超过 max_count 且不超过实际区域数）
 */
u32 pmm_get_memory_regions(memory_region_type *regions, u32 max_count);

#endif