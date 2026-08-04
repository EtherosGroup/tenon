#ifndef KERNEL_ASM_H
#define KERNEL_ASM_H

#include "types.h"

/* I/O Port Operations (x86 only) */
u8  asm_inb(u16 port);
void asm_outb(u16 port, u8 value);
u16 asm_inw(u16 port);
void asm_outw(u16 port, u16 value);
u32 asm_inl(u16 port);
void asm_outl(u16 port, u32 value);
void asm_io_wait(void);

/* MSR Operations (x86 only) */
u64 asm_rdmsr(u32 msr);
void asm_wrmsr(u32 msr, u64 value);

/* Control Register Operations */
unsigned long asm_read_cr0(void);
unsigned long asm_read_cr2(void);
unsigned long asm_read_cr3(void);
unsigned long asm_read_cr4(void);
void asm_write_cr0(unsigned long value);
void asm_write_cr3(unsigned long value);
void asm_write_cr4(unsigned long value);

/* CPUID */
void asm_cpuid
(
    u32 leaf, u32 subleaf,
    u32 *eax, u32 *ebx,
    u32 *ecx, u32 *edx
);

/* Instruction Wrappers */
void asm_hlt(void);
void asm_halt(void);
void asm_cli(void);
void asm_sti(void);
void asm_mfence(void);
void asm_lfence(void);
void asm_sfence(void);
void asm_pause(void);
void asm_invlpg(void *addr);

/* RFLAGS/EFLAGS */
unsigned long asm_read_rflags(void);
void asm_write_rflags(unsigned long flags);

/* Timestamp Counter */
u64 asm_rdtsc(void);
u64 asm_rdtscp(u32 *aux);

/* Atomic Exchange */
u32 asm_xchg(volatile u32 *ptr, u32 value);

/* Descriptor Table Operations */
void asm_lgdt(void *ptr, u16 size);
void asm_lidt(void *ptr, u16 size);
void asm_ltr(u16 sel);

/* Segment Register Reads */
u16 asm_read_cs(void);
u16 asm_read_ds(void);
u16 asm_read_es(void);
u16 asm_read_fs(void);
u16 asm_read_gs(void);
u16 asm_read_ss(void);

#endif
