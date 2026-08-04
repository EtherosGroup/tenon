#include "vmm.h"
#include "pmm.h"
#include "memlayout.h"
#include "serial.h"
#include "asm.h"

address_space_type kernel_as;

static void pt_zero_page(u64 phys) {
    // 通过恒等映射访问，phys 在 0-1 GiB 内，vmm_init 期间调用
    u64 *ptr = (u64 *)phys;
    for (int i = 0; i < 512; i++) ptr[i] = 0;
}

static void pt_zero_page_direct(u64 phys) {
    // 通过直接映射窗口访问，CR3 切换后调用
    u64 *ptr = (u64 *)PHYS_TO_DIRECT(phys);
    for (int i = 0; i < 512; i++) ptr[i] = 0;
}

void vmm_init(void) {
    /*
     * 此时 CR3 仍指向 boot.S 中的 PML4
     * 第 1 GiB 有恒等映射，所以 phys < 1 GiB 的页面可以直接用物理地址当指针访问
     * 所有页表结构页面通过 pmm_alloc_page_below(1 GiB) 分配，确保可访问
     */

    // 分配新 PML4
    u64 pml4_phys = pmm_alloc_page_below(0x40000000ULL);
    u64 *pml4 = (u64 *)pml4_phys;
    pt_zero_page(pml4_phys);

    // 恒等映射前 1 GiB（与 boot.S 相同）
    u64 pdpt_phys = pmm_alloc_page_below(0x40000000ULL);
    u64 *pdpt = (u64 *)pdpt_phys;
    pt_zero_page(pdpt_phys);

    u64 pd_phys = pmm_alloc_page_below(0x40000000ULL);
    u64 *pd = (u64 *)pd_phys;
    for (int i = 0; i < 512; i++) {
        pd[i] = (u64)i * 0x200000ULL | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE;
    }

    pdpt[0] = pd_phys | PTE_PRESENT | PTE_WRITABLE;
    pml4[0] = pdpt_phys | PTE_PRESENT | PTE_WRITABLE;

    // 递归页表映射: P4[510] -> PML4 自身
    pml4[RECURSIVE_PML4_IDX] = pml4_phys | PTE_PRESENT | PTE_WRITABLE;

    // 直接物理映射窗口
    u64 dm_p4_idx = PML4_IDX(KERNEL_DIRECT_BASE);
    u64 dm_pdpt_phys = pmm_alloc_page_below(0x40000000ULL);
    u64 *dm_pdpt = (u64 *)dm_pdpt_phys;
    pt_zero_page(dm_pdpt_phys);
    pml4[dm_p4_idx] = dm_pdpt_phys | PTE_PRESENT | PTE_WRITABLE;

    memory_region_type regions[MAX_MEMORY_REGIONS];
    u32 region_count = pmm_get_memory_regions(regions, MAX_MEMORY_REGIONS);

    for (u32 r = 0; r < region_count; r++) {
        u64 base = ALIGN_DOWN_2M(regions[r].base);
        u64 end  = regions[r].base + regions[r].length;

        for (u64 addr = base; addr < end; addr += 0x200000ULL) {
            u64 vaddr = KERNEL_DIRECT_BASE + addr;
            u64 pi = PDPT_IDX(vaddr);
            u64 di = PD_IDX(vaddr);

            if (!(dm_pdpt[pi] & PTE_PRESENT)) {
                u64 new_pd_phys = pmm_alloc_page_below(0x40000000ULL);
                pt_zero_page(new_pd_phys);
                dm_pdpt[pi] = new_pd_phys | PTE_PRESENT | PTE_WRITABLE;
            }

            u64 *cur_pd = (u64 *)(dm_pdpt[pi] & PTE_ADDR_MASK);
            cur_pd[di] = addr | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE | PTE_NX;
        }
    }

    // 切换 CR3 到新 PML4
    asm_write_cr3(pml4_phys);

    kernel_as.pml4_phys = pml4_phys;

    kprintln("[VMM] Initialized.");
}

void vmm_map_page(address_space_type *as, u64 va, u64 pa, u64 flags) {
    u64 p4 = PML4_IDX(va);
    u64 p3 = PDPT_IDX(va);
    u64 p2 = PD_IDX(va);
    u64 p1 = PT_IDX(va);

    u64 *pml4 = (u64 *)PHYS_TO_DIRECT(as->pml4_phys);

    if (!(pml4[p4] & PTE_PRESENT)) {
        u64 pt_phys = pmm_alloc_page();
        if (!pt_phys) return;
        pt_zero_page_direct(pt_phys);
        pml4[p4] = pt_phys | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    }

    u64 *pdpt = (u64 *)PHYS_TO_DIRECT(pml4[p4] & PTE_ADDR_MASK);

    if (pdpt[p3] & PTE_HUGE) return;

    if (!(pdpt[p3] & PTE_PRESENT)) {
        u64 pt_phys = pmm_alloc_page();
        if (!pt_phys) return;
        pt_zero_page_direct(pt_phys);
        pdpt[p3] = pt_phys | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    }

    u64 *pd = (u64 *)PHYS_TO_DIRECT(pdpt[p3] & PTE_ADDR_MASK);

    if (pd[p2] & PTE_HUGE) return;

    if (!(pd[p2] & PTE_PRESENT)) {
        u64 pt_phys = pmm_alloc_page();
        if (!pt_phys) return;
        pt_zero_page_direct(pt_phys);
        pd[p2] = pt_phys | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    }

    u64 *pt = (u64 *)PHYS_TO_DIRECT(pd[p2] & PTE_ADDR_MASK);

    pt[p1] = (pa & PTE_ADDR_MASK) | PTE_PRESENT | (flags & (PTE_WRITABLE | PTE_USER | PTE_NX | PTE_GLOBAL));

    asm_invlpg((void *)va);
}

void vmm_unmap_page(address_space_type *as, u64 va) {
    u64 p4 = PML4_IDX(va);
    u64 p3 = PDPT_IDX(va);
    u64 p2 = PD_IDX(va);
    u64 p1 = PT_IDX(va);

    u64 *pml4 = (u64 *)PHYS_TO_DIRECT(as->pml4_phys);
    if (!(pml4[p4] & PTE_PRESENT)) return;

    u64 *pdpt = (u64 *)PHYS_TO_DIRECT(pml4[p4] & PTE_ADDR_MASK);
    if (!(pdpt[p3] & PTE_PRESENT)) return;
    if (pdpt[p3] & PTE_HUGE) return;

    u64 *pd = (u64 *)PHYS_TO_DIRECT(pdpt[p3] & PTE_ADDR_MASK);
    if (!(pd[p2] & PTE_PRESENT)) return;
    if (pd[p2] & PTE_HUGE) return;

    u64 *pt = (u64 *)PHYS_TO_DIRECT(pd[p2] & PTE_ADDR_MASK);
    pt[p1] = 0;

    asm_invlpg((void *)va);
}

u64 vmm_get_mapping(address_space_type *as, u64 va) {
    u64 p4 = PML4_IDX(va);
    u64 p3 = PDPT_IDX(va);
    u64 p2 = PD_IDX(va);
    u64 p1 = PT_IDX(va);

    u64 *pml4 = (u64 *)PHYS_TO_DIRECT(as->pml4_phys);
    if (!(pml4[p4] & PTE_PRESENT)) return 0;


    u64 *pdpt = (u64 *)PHYS_TO_DIRECT(pml4[p4] & PTE_ADDR_MASK);
    if (!(pdpt[p3] & PTE_PRESENT)) return 0;

    u64 *pd = (u64 *)PHYS_TO_DIRECT(pdpt[p3] & PTE_ADDR_MASK);
    if (!(pd[p2] & PTE_PRESENT)) return 0;

    if (pd[p2] & PTE_HUGE) {
        return (pd[p2] & PTE_ADDR_MASK) | (va & 0x1FFFFFULL);
    }

    u64 *pt = (u64 *)PHYS_TO_DIRECT(pd[p2] & PTE_ADDR_MASK);
    if (!(pt[p1] & PTE_PRESENT)) return 0;

    return (pt[p1] & PTE_ADDR_MASK) | (va & PAGE_MASK);
}