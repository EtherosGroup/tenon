#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include "types.h"

#define IDT_GATE_INTERRUPT 0x8E // 中断门
#define IDT_GATE_TRAP 0x8F // 陷阱门
#define KERNEL_CS 0x08 // 选择子
#define IRQ_BASE 0x20 // IQR基址

typedef struct 
{
    u64 low;
    u64 high;
} __attribute__((packed)) idt_gate_type;

void idt_set_gate(u8 vector, void (*isr)(void), u8 type, u8 dpl, u8 ist);
void idt_init(void);

typedef struct {
    u64 rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    u64 vector;
    u64 error_code;
    u64 rip, cs, rflags, rsp, ss;
} __attribute__((packed)) int_frame_type;

void isr_handler(int_frame_type *frame);

#endif