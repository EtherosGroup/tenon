#ifndef KERNEL_ASM_H
#define KERNEL_ASM_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

/* I/O Port Operations (x86 only) */
uint8_t  asm_inb(uint16_t port);
void     asm_outb(uint16_t port, uint8_t value);
uint16_t asm_inw(uint16_t port);
void     asm_outw(uint16_t port, uint16_t value);
uint32_t asm_inl(uint16_t port);
void     asm_outl(uint16_t port, uint32_t value);
void     asm_io_wait(void);

/* MSR Operations (x86 only) */
uint64_t asm_rdmsr(uint32_t msr);
void     asm_wrmsr(uint32_t msr, uint64_t value);

/* Control Register Operations */
unsigned long asm_read_cr0(void);
unsigned long asm_read_cr2(void);
unsigned long asm_read_cr3(void);
unsigned long asm_read_cr4(void);
void          asm_write_cr0(unsigned long value);
void          asm_write_cr3(unsigned long value);
void          asm_write_cr4(unsigned long value);

/* CPUID */
void asm_cpuid(uint32_t leaf, uint32_t subleaf,
               uint32_t *eax, uint32_t *ebx,
               uint32_t *ecx, uint32_t *edx);

/* Instruction Wrappers */
void asm_hlt(void);
void asm_cli(void);
void asm_sti(void);
void asm_mfence(void);
void asm_lfence(void);
void asm_sfence(void);
void asm_pause(void);
void asm_invlpg(void *addr);

/* RFLAGS/EFLAGS */
unsigned long asm_read_rflags(void);
void          asm_write_rflags(unsigned long flags);

/* Timestamp Counter */
uint64_t asm_rdtsc(void);
uint64_t asm_rdtscp(uint32_t *aux);

/* Atomic Exchange */
uint32_t asm_xchg(volatile uint32_t *ptr, uint32_t value);

/* Descriptor Table Operations */
void asm_lgdt(void *ptr, uint16_t size);
void asm_lidt(void *ptr, uint16_t size);
void asm_ltr(uint16_t sel);

/* Segment Register Reads */
uint16_t asm_read_cs(void);
uint16_t asm_read_ds(void);
uint16_t asm_read_es(void);
uint16_t asm_read_fs(void);
uint16_t asm_read_gs(void);
uint16_t asm_read_ss(void);

#endif /* KERNEL_ASM_H */
