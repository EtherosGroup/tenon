#include "asm.h"

/* ========================================================================
 * x86 Implementation
 * ======================================================================== */
#if defined(__x86_64__) || defined(__i386__)

ku8 asm_inb(ku16 port)
{
    ku8 value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outb(ku16 port, ku8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

ku16 asm_inw(ku16 port)
{
    ku16 value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outw(ku16 port, ku16 value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

ku32 asm_inl(ku16 port)
{
    ku32 value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void asm_outl(ku16 port, ku32 value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

void asm_io_wait(void)
{
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

ku64 asm_rdmsr(ku32 msr)
{
    ku32 low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((ku64)high << 32) | low;
}

void asm_wrmsr(ku32 msr, ku64 value)
{
    ku32 low = (ku32)value;
    ku32 high = (ku32)(value >> 32);
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

void asm_cpuid(ku32 leaf, ku32 subleaf,
               ku32 *eax, ku32 *ebx,
               ku32 *ecx, ku32 *edx)
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

ku64 asm_rdtsc(void)
{
    ku32 low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((ku64)high << 32) | low;
}

ku64 asm_rdtscp(ku32 *aux)
{
    ku32 low, high;
    __asm__ volatile("rdtscp" : "=a"(low), "=d"(high), "=c"(*aux));
    return ((ku64)high << 32) | low;
}

ku32 asm_xchg(volatile ku32 *ptr, ku32 value)
{
    __asm__ volatile("xchgl %0, %1"
        : "+r"(value), "+m"(*ptr)
        :
        : "memory");
    return value;
}

void asm_lgdt(void *ptr, ku16 size)
{
    struct {
        ku16 limit;
        unsigned long base;
    } __attribute__((packed)) gdtr;

    gdtr.limit = size - 1;
    gdtr.base = (unsigned long)ptr;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));
}

void asm_lidt(void *ptr, ku16 size)
{
    struct {
        ku16 limit;
        unsigned long base;
    } __attribute__((packed)) idtr;

    idtr.limit = size - 1;
    idtr.base = (unsigned long)ptr;

    __asm__ volatile("lidt %0" : : "m"(idtr));
}

void asm_ltr(ku16 sel)
{
    __asm__ volatile("ltr %0" : : "r"((unsigned long)sel));
}

ku16 asm_read_cs(void)
{
    ku16 cs;
    __asm__ volatile("mov %%cs, %0" : "=r"(cs));
    return cs;
}

ku16 asm_read_ds(void)
{
    ku16 ds;
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    return ds;
}

ku16 asm_read_es(void)
{
    ku16 es;
    __asm__ volatile("mov %%es, %0" : "=r"(es));
    return es;
}

ku16 asm_read_fs(void)
{
    ku16 fs;
    __asm__ volatile("mov %%fs, %0" : "=r"(fs));
    return fs;
}

ku16 asm_read_gs(void)
{
    ku16 gs;
    __asm__ volatile("mov %%gs, %0" : "=r"(gs));
    return gs;
}

ku16 asm_read_ss(void)
{
    ku16 ss;
    __asm__ volatile("mov %%ss, %0" : "=r"(ss));
    return ss;
}

/* ========================================================================
 * ARM Stubs
 * ======================================================================== */
#elif defined(__arm__) || defined(__aarch64__)

ku8 asm_inb(ku16 port) { (void)port; return 0; }
void asm_outb(ku16 port, ku8 value) { (void)port; (void)value; }
ku16 asm_inw(ku16 port) { (void)port; return 0; }
void asm_outw(ku16 port, ku16 value) { (void)port; (void)value; }
ku32 asm_inl(ku16 port) { (void)port; return 0; }
void asm_outl(ku16 port, ku32 value) { (void)port; (void)value; }
void asm_io_wait(void) {}

ku64 asm_rdmsr(ku32 msr) { (void)msr; return 0; }
void asm_wrmsr(ku32 msr, ku64 value) { (void)msr; (void)value; }

unsigned long asm_read_cr0(void) { return 0; }
unsigned long asm_read_cr2(void) { return 0; }
unsigned long asm_read_cr3(void) { return 0; }
unsigned long asm_read_cr4(void) { return 0; }
void asm_write_cr0(unsigned long value) { (void)value; }
void asm_write_cr3(unsigned long value) { (void)value; }
void asm_write_cr4(unsigned long value) { (void)value; }

void asm_cpuid(ku32 leaf, ku32 subleaf,
               ku32 *eax, ku32 *ebx,
               ku32 *ecx, ku32 *edx)
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

ku64 asm_rdtsc(void) { return 0; }
ku64 asm_rdtscp(ku32 *aux)
{
    if (aux) *aux = 0;
    return 0;
}

ku32 asm_xchg(volatile ku32 *ptr, ku32 value)
{
    ku32 old = *ptr;
    *ptr = value;
    return old;
}

void asm_lgdt(void *ptr, ku16 size) { (void)ptr; (void)size; }
void asm_lidt(void *ptr, ku16 size) { (void)ptr; (void)size; }
void asm_ltr(ku16 sel) { (void)sel; }

ku16 asm_read_cs(void) { return 0; }
ku16 asm_read_ds(void) { return 0; }
ku16 asm_read_es(void) { return 0; }
ku16 asm_read_fs(void) { return 0; }
ku16 asm_read_gs(void) { return 0; }
ku16 asm_read_ss(void) { return 0; }

#else
#error "Unsupported architecture"
#endif
