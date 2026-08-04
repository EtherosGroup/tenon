.code64

# void task_switch(task_context_type *old, task_context_type *new)
# rdi = old, rsi = new
.global task_switch
task_switch:
    # 保存当前 callee-saved 寄存器到 *old
    mov %r15, 0(%rdi)
    mov %r14, 8(%rdi)
    mov %r13, 16(%rdi)
    mov %r12, 24(%rdi)
    mov %rbx, 32(%rdi)
    mov %rbp, 40(%rdi)
    mov %rsp, 48(%rdi)

    # 从 *new 加载新任务的 callee-saved 寄存器
    mov 0(%rsi), %r15
    mov 8(%rsi), %r14
    mov 16(%rsi), %r13
    mov 24(%rsi), %r12
    mov 32(%rsi), %rbx
    mov 40(%rsi), %rbp
    mov 48(%rsi), %rsp

    ret

# 新任务首次启动入口
.global task_start
task_start:
    sti
    pop %rdi
    pop %rsi
    call *%rdi
    call task_exit