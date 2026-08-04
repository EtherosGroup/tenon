#include "idt.h"
#include "asm.h"
#include "serial.h"

static idt_gate_type idt[256] __attribute__((aligned(4096)));

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);

extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr32(void);
extern void isr33(void);
extern void isr34(void);
extern void isr35(void);
extern void isr36(void);
extern void isr37(void);
extern void isr38(void);
extern void isr39(void);
extern void isr40(void);
extern void isr41(void);
extern void isr42(void);
extern void isr43(void);
extern void isr44(void);
extern void isr45(void);
extern void isr46(void);
extern void isr47(void);

static void (*isr_table[48])(void) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39,
    isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47,
};

static void default_exception_handler(void)
{
    kprintln("Exception!");
    asm_halt();
}

void idt_set_gate(u8 vector, void (*isr)(void), u8 type, u8 dpl, u8 ist)
{
    u64 addr = (u64)isr;

    idt[vector].low  = (addr & 0xFFFF)
                     | ((u64)KERNEL_CS << 16)
                     | ((u64)(ist & 7) << 32)
                     | ((u64)(type & 0xF) << 40)
                     | ((u64)(dpl & 3) << 45)
                     | (1ULL << 47)
                     | ((addr >> 16) & 0xFFFF) << 48;

    idt[vector].high = addr >> 32;
}

void idt_init(void)
{
    for (u16 i = 0; i < 48; i++)
    {
        idt_set_gate((u8)i, isr_table[i], IDT_GATE_INTERRUPT, 0, 0);
    }

    for (u16 i = 48; i < 256; i++)
    {
        idt_set_gate((u8)i, default_exception_handler, IDT_GATE_INTERRUPT, 0, 0);
    }

    idt_set_gate(2,  isr_table[2],  IDT_GATE_INTERRUPT, 0, 1);   // NMI → IST1
    idt_set_gate(8,  isr_table[8],  IDT_GATE_INTERRUPT, 0, 2);   // Double Fault → IST2
    idt_set_gate(18, isr_table[18], IDT_GATE_INTERRUPT, 0, 3);   // Machine Check → IST3

    asm_lidt(idt, sizeof(idt));
}
