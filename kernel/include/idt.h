#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include "types.h"

#define IDT_GATE_INTERRUPT 0x8E // 中断门
#define IDT_GATE_TRAP 0x8F // 陷阱门
#define KERNEL_CS 0x08 // 选择子
#define IRQ_BASE 0x20 // IQR基址

typedef struct 
{
    ku64 low;
    ku64 high;
} __attribute__((packed)) idt_gate_type;

void idt_set_gate(ku8 vector, void (*isr)(void), ku8 type, ku8 dpl);
void idt_init(void);

typedef struct {
    ku64 rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    ku64 vector;
    ku64 error_code;
    ku64 rip, cs, rflags, rsp, ss;
} __attribute__((packed)) int_frame_type;

void isr_handler(int_frame_type *frame);

#endif