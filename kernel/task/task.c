#include "task.h"
#include "asm.h"
#include "serial.h"
#include "kheap.h"
#include "memlayout.h"

task_type *current_task;
task_type *ready_queue;
static ku32 next_task_id = 1;

void task_init(void)
{
    task_type *idle = (task_type *)kmalloc(sizeof(task_type));
    if (!idle) {
        kprintln("[TASK] Failed to allocate idle task");
        asm_halt();
    }

    idle->id = 0;
    idle->state = TASK_RUNNING;
    idle->kernel_stack = NULL;
    idle->name = "idle";
    idle->next = idle;

    current_task = idle;
    ready_queue = idle;
    next_task_id = 1;

    kprintln("[TASK] Initialized.");
}

task_type *task_create(void (*entry)(void), const char *name)
{
    task_type *task = (task_type *)kmalloc(sizeof(task_type));
    if (!task) {
        kprintln("[TASK] Failed to allocate task struct");
        return NULL;
    }

    ku8 *stack = (ku8 *)kmalloc(PAGE_SIZE);
    if (!stack) {
        kprintln("[TASK] Failed to allocate kernel stack");
        kfree(task);
        return NULL;
    }

    ku64 stack_top = (ku64)stack + PAGE_SIZE;
    ku64 *sp = (ku64 *)(stack_top - 16);

    sp[0] = (ku64)task_start; // task_switch 的 ret 目标
    sp[1] = (ku64)entry; // trampoline 的 pop %rdi 目标

    task->id = next_task_id++;
    task->state = TASK_READY;
    task->context.r15 = 0;
    task->context.r14 = 0;
    task->context.r13 = 0;
    task->context.r12 = 0;
    task->context.rbx = 0;
    task->context.rbp = 0;
    task->context.rsp = stack_top - 16;
    task->kernel_stack = stack;
    task->name = name;

    task->next = ready_queue->next;
    ready_queue->next = task;

    return task;
}

void task_exit(void)
{
    current_task->state = TASK_DEAD;

    task_type *prev = ready_queue;
    while (prev->next != current_task)
    {
        prev = prev->next;
    }
    prev->next = current_task->next;

    // TODO: 释放 kernel_stack 和 task 结构体内存（需要 reaper 机制）
    schedule();

    // unreachable
    for (;;) asm_hlt();
}

void task_yield(void)
{
    current_task->state = TASK_READY;
    schedule();
}