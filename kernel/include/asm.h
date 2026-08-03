#ifndef KERNEL_ASM_H
#define KERNEL_ASM_H

#include "types.h"

/* I/O Port Operations (x86 only) */
ku8  asm_inb(ku16 port);
void asm_outb(ku16 port, ku8 value);
ku16 asm_inw(ku16 port);
void asm_outw(ku16 port, ku16 value);
ku32 asm_inl(ku16 port);
void asm_outl(ku16 port, ku32 value);
void asm_io_wait(void);

/* MSR Operations (x86 only) */
ku64 asm_rdmsr(ku32 msr);
void asm_wrmsr(ku32 msr, ku64 value);

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
    ku32 leaf, ku32 subleaf,
    ku32 *eax, ku32 *ebx,
    ku32 *ecx, ku32 *edx
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
ku64 asm_rdtsc(void);
ku64 asm_rdtscp(ku32 *aux);

/* Atomic Exchange */
ku32 asm_xchg(volatile ku32 *ptr, ku32 value);

/* Descriptor Table Operations */
void asm_lgdt(void *ptr, ku16 size);
void asm_lidt(void *ptr, ku16 size);
void asm_ltr(ku16 sel);

/* Segment Register Reads */
ku16 asm_read_cs(void);
ku16 asm_read_ds(void);
ku16 asm_read_es(void);
ku16 asm_read_fs(void);
ku16 asm_read_gs(void);
ku16 asm_read_ss(void);

#endif
