#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include "types.h"

// 页表条目标志
#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_PWT (1ULL << 3)
#define PTE_PCD (1ULL << 4)
#define PTE_ACCESSED (1ULL << 5)
#define PTE_DIRTY (1ULL << 6)
#define PTE_HUGE (1ULL << 7)
#define PTE_GLOBAL (1ULL << 8)
#define PTE_NX (1ULL << 63)

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

// 虚拟地址 -> 页表索引
#define PML4_IDX(va) (((u64)(va) >> 39) & 0x1FF)
#define PDPT_IDX(va) (((u64)(va) >> 30) & 0x1FF)
#define PD_IDX(va) (((u64)(va) >> 21) & 0x1FF)
#define PT_IDX(va) (((u64)(va) >> 12) & 0x1FF)

// 地址空间抽象
typedef struct
{
    u64  pml4_phys;
    u64 *pml4; // 通过递归映射访问的虚拟地址
} address_space_type;

extern address_space_type kernel_as;

/**
 * 初始化
 */
void vmm_init(void);

/**
 * 建立虚拟地址到物理地址的页表映射
 */
void vmm_map_page(address_space_type *as, u64 va, u64 pa, u64 flags);

/**
 * 解除虚拟地址的页表映射
 */
void vmm_unmap_page(address_space_type *as, u64 va);

/**
 * 查询虚拟地址对应的物理地址和权限
 */
u64 vmm_get_mapping(address_space_type *as, u64 va);

#endif