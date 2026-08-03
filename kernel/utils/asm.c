#include "asm.h"

/* ========================================================================
 * x86 Implementation
 * ======================================================================== */
#if defined(__x86_64__) || defined(__i386__)

uint8_t asm_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t asm_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t asm_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

void asm_io_wait(void)
{
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

uint64_t asm_rdmsr(uint32_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void asm_wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

unsigned long asm_read_cr0(void)
{
    unsigned long cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

unsigned long asm_read_cr2(void)
{
    unsigned long cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

unsigned long asm_read_cr3(void)
{
    unsigned long cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

unsigned long asm_read_cr4(void)
{
    unsigned long cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

void asm_write_cr0(unsigned long value)
{
    __asm__ volatile("mov %0, %%cr0" : : "r"(value));
}

void asm_write_cr3(unsigned long value)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(value));
}

void asm_write_cr4(unsigned long value)
{
    __asm__ volatile("mov %0, %%cr4" : : "r"(value));
}

void asm_cpuid(uint32_t leaf, uint32_t subleaf,
               uint32_t *eax, uint32_t *ebx,
               uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf));
}

void asm_hlt(void)
{
    __asm__ volatile("hlt");
}

void asm_cli(void)
{
    __asm__ volatile("cli");
}

void asm_sti(void)
{
    __asm__ volatile("sti");
}

void asm_mfence(void)
{
    __asm__ volatile("mfence" : : : "memory");
}

void asm_lfence(void)
{
    __asm__ volatile("lfence" : : : "memory");
}

void asm_sfence(void)
{
    __asm__ volatile("sfence" : : : "memory");
}

void asm_pause(void)
{
    __asm__ volatile("pause");
}

void asm_invlpg(void *addr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

unsigned long asm_read_rflags(void)
{
    unsigned long flags;
    __asm__ volatile(
        "pushf\n\t"
        "pop %0"
        : "=r"(flags)
        :
        : "memory");
    return flags;
}

void asm_write_rflags(unsigned long flags)
{
    __asm__ volatile(
        "push %0\n\t"
        "popf"
        :
        : "r"(flags)
        : "memory", "cc");
}

uint64_t asm_rdtsc(void)
{
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t asm_rdtscp(uint32_t *aux)
{
    uint32_t low, high;
    __asm__ volatile("rdtscp" : "=a"(low), "=d"(high), "=c"(*aux));
    return ((uint64_t)high << 32) | low;
}

uint32_t asm_xchg(volatile uint32_t *ptr, uint32_t value)
{
    __asm__ volatile("xchgl %0, %1"
        : "+r"(value), "+m"(*ptr)
        :
        : "memory");
    return value;
}

void asm_lgdt(void *ptr, uint16_t size)
{
    struct {
        uint16_t limit;
        unsigned long base;
    } __attribute__((packed)) gdtr;

    gdtr.limit = size - 1;
    gdtr.base = (unsigned long)ptr;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));
}

void asm_lidt(void *ptr, uint16_t size)
{
    struct {
        uint16_t limit;
        unsigned long base;
    } __attribute__((packed)) idtr;

    idtr.limit = size - 1;
    idtr.base = (unsigned long)ptr;

    __asm__ volatile("lidt %0" : : "m"(idtr));
}

void asm_ltr(uint16_t sel)
{
    __asm__ volatile("ltr %0" : : "r"((unsigned long)sel));
}

uint16_t asm_read_cs(void)
{
    uint16_t cs;
    __asm__ volatile("mov %%cs, %0" : "=r"(cs));
    return cs;
}

uint16_t asm_read_ds(void)
{
    uint16_t ds;
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    return ds;
}

uint16_t asm_read_es(void)
{
    uint16_t es;
    __asm__ volatile("mov %%es, %0" : "=r"(es));
    return es;
}

uint16_t asm_read_fs(void)
{
    uint16_t fs;
    __asm__ volatile("mov %%fs, %0" : "=r"(fs));
    return fs;
}

uint16_t asm_read_gs(void)
{
    uint16_t gs;
    __asm__ volatile("mov %%gs, %0" : "=r"(gs));
    return gs;
}

uint16_t asm_read_ss(void)
{
    uint16_t ss;
    __asm__ volatile("mov %%ss, %0" : "=r"(ss));
    return ss;
}

/* ========================================================================
 * ARM Stubs
 * ======================================================================== */
#elif defined(__arm__) || defined(__aarch64__)

uint8_t asm_inb(uint16_t port) { (void)port; return 0; }
void asm_outb(uint16_t port, uint8_t value) { (void)port; (void)value; }
uint16_t asm_inw(uint16_t port) { (void)port; return 0; }
void asm_outw(uint16_t port, uint16_t value) { (void)port; (void)value; }
uint32_t asm_inl(uint16_t port) { (void)port; return 0; }
void asm_outl(uint16_t port, uint32_t value) { (void)port; (void)value; }
void asm_io_wait(void) {}

uint64_t asm_rdmsr(uint32_t msr) { (void)msr; return 0; }
void asm_wrmsr(uint32_t msr, uint64_t value) { (void)msr; (void)value; }

unsigned long asm_read_cr0(void) { return 0; }
unsigned long asm_read_cr2(void) { return 0; }
unsigned long asm_read_cr3(void) { return 0; }
unsigned long asm_read_cr4(void) { return 0; }
void asm_write_cr0(unsigned long value) { (void)value; }
void asm_write_cr3(unsigned long value) { (void)value; }
void asm_write_cr4(unsigned long value) { (void)value; }

void asm_cpuid(uint32_t leaf, uint32_t subleaf,
               uint32_t *eax, uint32_t *ebx,
               uint32_t *ecx, uint32_t *edx)
{
    (void)leaf; (void)subleaf;
    if (eax) *eax = 0;
    if (ebx) *ebx = 0;
    if (ecx) *ecx = 0;
    if (edx) *edx = 0;
}

void asm_hlt(void) {}
void asm_cli(void) {}
void asm_sti(void) {}
void asm_mfence(void) {}
void asm_lfence(void) {}
void asm_sfence(void) {}
void asm_pause(void) {}
void asm_invlpg(void *addr) { (void)addr; }

unsigned long asm_read_rflags(void) { return 0; }
void asm_write_rflags(unsigned long flags) { (void)flags; }

uint64_t asm_rdtsc(void) { return 0; }
uint64_t asm_rdtscp(uint32_t *aux)
{
    if (aux) *aux = 0;
    return 0;
}

uint32_t asm_xchg(volatile uint32_t *ptr, uint32_t value)
{
    uint32_t old = *ptr;
    *ptr = value;
    return old;
}

void asm_lgdt(void *ptr, uint16_t size) { (void)ptr; (void)size; }
void asm_lidt(void *ptr, uint16_t size) { (void)ptr; (void)size; }
void asm_ltr(uint16_t sel) { (void)sel; }

uint16_t asm_read_cs(void) { return 0; }
uint16_t asm_read_ds(void) { return 0; }
uint16_t asm_read_es(void) { return 0; }
uint16_t asm_read_fs(void) { return 0; }
uint16_t asm_read_gs(void) { return 0; }
uint16_t asm_read_ss(void) { return 0; }

#else
#error "Unsupported architecture"
#endif
