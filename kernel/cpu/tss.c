#include "tss.h"
#include "asm.h"
#include "serial.h"
#include "kheap.h"
#include "memlayout.h"

void tss_init(void)
{
    tss_type *tss = (tss_type *)kmalloc(sizeof(tss_type));
    if (!tss) {
        kprintln("[TSS] Failed to allocate TSS");
        asm_halt();
    }

    for (u32 i = 0; i < sizeof(tss_type); i++)
        ((u8 *)tss)[i] = 0;
    tss->iopb_offset = sizeof(tss_type);

    u8 *ist1_stack = (u8 *)kmalloc(PAGE_SIZE);
    u8 *ist2_stack = (u8 *)kmalloc(PAGE_SIZE);
    u8 *ist3_stack = (u8 *)kmalloc(PAGE_SIZE);

    if (!ist1_stack || !ist2_stack || !ist3_stack) {
        kprintln("[TSS] Failed to allocate IST stacks");
        asm_halt();
    }

    tss->ist1 = (u64)ist1_stack + PAGE_SIZE;
    tss->ist2 = (u64)ist2_stack + PAGE_SIZE;
    tss->ist3 = (u64)ist3_stack + PAGE_SIZE;

    u64 *gdt = (u64 *)kmalloc(4 * 8);
    if (!gdt) {
        kprintln("[TSS] Failed to allocate GDT");
        asm_halt();
    }

    u64 tss_addr  = (u64)tss;
    u64 tss_limit = sizeof(tss_type) - 1;

    gdt[0] = 0;

    gdt[1] = (1ULL << 41) | (1ULL << 43) | (1ULL << 44)
           | (1ULL << 47) | (1ULL << 53);

    gdt[2] = (tss_limit & 0xFFFF)
           | ((tss_addr & 0xFFFFFF) << 16)
           | (0x9ULL << 40)
           | (1ULL << 47)
           | (((tss_limit >> 16) & 0xF) << 48)
           | (((tss_addr >> 24) & 0xFF) << 56);

    gdt[3] = tss_addr >> 32;

    asm_lgdt(gdt, 4 * 8);
    asm_ltr(0x10);

    kprintln("[TSS] Initialized.");
}