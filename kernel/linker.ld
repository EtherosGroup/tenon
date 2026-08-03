OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_PHYS = 1M;

PHDRS
{
    text PT_LOAD;
    bss  PT_LOAD;
    note PT_NOTE;
}

SECTIONS
{
    . = KERNEL_PHYS;

    .multiboot : {
        *(.multiboot)
    } :text

    .note : {
        *(.note.Xen)
    } :note

    .text : {
        *(.text*)
    } :text

    .rodata : {
        *(.rodata*)
    } :text

    .data : {
        *(.data*)
    } :text

    .bss : {
        *(.bss*)
    } :bss

    /DISCARD/ : {
        *(.comment*)
        *(.eh_frame*)
    }
}
