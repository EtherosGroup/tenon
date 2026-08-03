#include "idt.h"
#include "asm.h"
#include "serial.h"

static idt_gate_type idt[256] __attribute__((aligned(4096)));

static void default_exception_handler(void)
{
    kprintln("Exception!");
    asm_halt();
}

void idt_set_gate(ku8 vector, void (*isr)(void), ku8 type, ku8 dpl)
{
    ku64 addr = (ku64)isr;

    idt[vector].low  = (addr & 0xFFFF)
                     | ((ku64)KERNEL_CS << 16)
                     | ((ku64)(type & 0xF) << 40)
                     | ((ku64)(dpl & 3) << 45)
                     | (1ULL << 47)
                     | ((addr >> 16) & 0xFFFF) << 48;

    idt[vector].high = addr >> 32;
}

void idt_init(void)
{
    for (ku16 i = 0; i < 256; i++)
        idt_set_gate(i, default_exception_handler, IDT_GATE_INTERRUPT, 0);

    asm_lidt(idt, sizeof(idt));
}
