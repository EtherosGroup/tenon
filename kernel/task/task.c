#include "task.h"
#include "asm.h"
#include "serial.h"
#include "kheap.h"
#include "memlayout.h"
#include "pit.h"

task_type *current_task;
task_type *ready_queue;
static u32 next_task_id = 1;
static task_type *dead_list = null;

void task_init(void)
{
    task_type *idle = (task_type *)kmalloc(sizeof(task_type));
    if (!idle)
    {
        kprintln("[TASK] Failed to allocate idle task");
        asm_halt();
    }

    idle->id = 0;
    idle->state = TASK_RUNNING;
    idle->kernel_stack = null;
    idle->name = "idle";
    idle->next = idle;
    idle->sleep_until = 0;

    current_task = idle;
    ready_queue = idle;
    next_task_id = 1;
    dead_list = null;

    kprintln("[TASK] Initialized.");
}

task_type *task_create(void (*entry)(void *arg), void *arg, const char *name)
{
    task_type *task = (task_type *)kmalloc(sizeof(task_type));
    if (!task)
    {
        kprintln("[TASK] Failed to allocate task struct");
        return null;
    }

    u8 *stack = (u8 *)kmalloc(PAGE_SIZE);
    if (!stack)
    {
        kprintln("[TASK] Failed to allocate kernel stack");
        kfree(task);
        return null;
    }

    u64 stack_top = (u64)stack + PAGE_SIZE;
    u64 *sp = (u64 *)(stack_top - 24);

    sp[0] = (u64)task_start;
    sp[1] = (u64)entry;
    sp[2] = (u64)arg;

    task->id = next_task_id++;
    task->state = TASK_READY;
    task->context.r15 = 0;
    task->context.r14 = 0;
    task->context.r13 = 0;
    task->context.r12 = 0;
    task->context.rbx = 0;
    task->context.rbp = 0;
    task->context.rsp = stack_top - 24;
    task->kernel_stack = stack;
    task->name = name;
    task->sleep_until = 0;

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

    current_task->next = dead_list;
    dead_list = current_task;

    schedule();

    for (;;)
    {
        asm_hlt();
    }
}

void task_yield(void)
{
    current_task->state = TASK_READY;
    schedule();
}

void task_block(void)
{
    current_task->state = TASK_BLOCKED;
    schedule();
}

void task_wakeup(task_type *task)
{
    if (task->state == TASK_BLOCKED)
    {
        task->state = TASK_READY;
        task->sleep_until = 0;
    }
}

void task_sleep_ms(u32 ms)
{
    current_task->sleep_until = tick_count + ms;
    task_block();
}

void reap_dead_tasks(void)
{
    while (dead_list)
    {
        task_type *dead = dead_list;
        dead_list = dead->next;
        kfree(dead->kernel_stack);
        kfree(dead);
    }
}