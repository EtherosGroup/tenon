#ifndef KERNEL_MEMLAYOUT_H
#define KERNEL_MEMLAYOUT_H

#include "types.h"

/**
 * 页常量
 */
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PAGE_MASK 0xFFFULL
#define PAGE_ALIGN_UP(addr) (((u64)(addr) + PAGE_MASK) & ~PAGE_MASK)
#define PAGE_ALIGN_DOWN(addr) ((u64)(addr) & ~PAGE_MASK)
#define ADDR_TO_PAGE(addr) ((u64)(addr) >> PAGE_SHIFT)
#define PAGE_TO_ADDR(page) ((u64)(page) << PAGE_SHIFT)

#define ALIGN_DOWN_2M(addr) ((u64)(addr) & ~0x1FFFFFULL)

/**
 * 内核物理边界，由 linker.ld 导出
 */
extern u8 __kernel_phys_start[];
extern u8 __kernel_phys_end[];

/**
 * 虚拟地址布局
 * 0x0000000000000000 ─ 0x00007FFFFFFFFFFF   用户空间，128 TiB
 * 0xFFFF800000000000 ─ 0xFFFFFFFFFFFFFFFF   内核空间，128 TiB
 *
 * 内核区域
 * 0xFFFF900000000000   KERNEL_HEAP_START   内核堆
 * 0xFFFFA00000000000   KERNEL_DIRECT_BASE   物理内存直接映射窗口
 * 0xFFFFFF0000000000   PGTBL_BASE   递归页表映射区域
 */

#define KERNEL_HEAP_START 0xFFFF900000000000ULL
#define KERNEL_HEAP_SIZE (64ULL * 1024ULL * 1024ULL)

#define KERNEL_DIRECT_BASE 0xFFFFA00000000000ULL
#define PHYS_TO_DIRECT(p) ((u64)(p) + KERNEL_DIRECT_BASE)
#define DIRECT_TO_PHYS(v) ((u64)(v) - KERNEL_DIRECT_BASE)

#define RECURSIVE_PML4_IDX 510
#define PGTBL_VADDR(p4, p3, p2, p1) (0xFFFFULL << 48 | (u64)(RECURSIVE_PML4_IDX) << 39 | (u64)(p4) << 30 | (u64)(p3) << 21 | (u64)(p2) << 12 | (u64)(p1) << 3)

#endif