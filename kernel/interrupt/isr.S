.code64

.macro SAVE_REGS
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
.endm

.macro RESTORE_REGS
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
.endm

# 异常无错误码
.macro ISR_NOERR n
.global isr\n
isr\n:
    push $0
    push $\n
    jmp  common_handler
.endm

# 异常有错误码
.macro ISR_ERRCODE n
.global isr\n
isr\n:
    push $\n
    jmp  common_handler
.endm

# 硬件中断
.macro IRQ n vec
.global isr\vec
isr\vec:
    push $0
    push $\vec
    jmp  common_handler
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERRCODE 8
ISR_NOERR 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERRCODE 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERRCODE 30
ISR_NOERR 31

IRQ 0  32
IRQ 1  33
IRQ 2  34
IRQ 3  35
IRQ 4  36
IRQ 5  37
IRQ 6  38
IRQ 7  39
IRQ 8  40
IRQ 9  41
IRQ 10 42
IRQ 11 43
IRQ 12 44
IRQ 13 45
IRQ 14 46
IRQ 15 47

common_handler:
    SAVE_REGS
    mov  %rsp, %rdi
    call isr_handler
    RESTORE_REGS
    add  $16, %rsp        # 跳过 vector + error_code
    iretq