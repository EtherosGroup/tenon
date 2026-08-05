.code32

.section .multiboot, "a"
.align 8
mb_header_start:
    .long 0xE85250D6
    .long 0
    .long (mb_header_end - mb_header_start)
    .long (0x100000000 - 0xE85250D6 - (mb_header_end - mb_header_start))

    .word 3
    .word 0
    .long 12
    .long _start
    .long 0

    .word 5
    .word 0
    .long 20
    .long 0
    .long 0
    .long 32
    .long 0

    .word 0
    .word 0
    .long 8
mb_header_end:

.section .bss
.align 4096
pml4:
    .space 4096
pdpt:
    .space 4096
pd:
    .space 4096

.align 16
stack_bottom:
    .space 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    movl    $stack_top, %esp

    movl    %eax, %edi
    movl    %ebx, %esi

    movl    $pdpt, %eax
    orl     $3, %eax
    movl    %eax, pml4

    movl    $pd, %eax
    orl     $3, %eax
    movl    %eax, pdpt

    xorl    %ecx, %ecx
1:
    movl    $0x200000, %eax
    mull    %ecx
    orl     $0x83, %eax
    movl    %eax, pd(, %ecx, 8)
    incl    %ecx
    cmpl    $512, %ecx
    jl      1b

    movl    $pml4, %eax
    movl    %eax, %cr3

    movl    %cr4, %eax
    orl     $(1 << 5), %eax
    movl    %eax, %cr4

    movl    $0xC0000080, %ecx
    rdmsr
    orl     $(1 << 8) | (1 << 11), %eax
    wrmsr

    movl    %cr0, %eax
    orl     $(1 << 31), %eax
    movl    %eax, %cr0

    lgdt    gdt64_ptr

    ljmp    $8, $long_mode_start

.code64
long_mode_start:
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss

    movl    %edi, %edi
    movl    %esi, %esi

    call    start_kernel

    hlt
1:  jmp     1b

.section .note.Xen, "", @note
    .long 4
    .long 4
    .long 18
    .asciz "Xen"
    .long _start

.section .rodata
.align 8
gdt64:
    .quad 0
    .quad (1 << 41) | (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
gdt64_ptr:
    .word (gdt64_ptr - gdt64 - 1)
    .quad gdt64
    