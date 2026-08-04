#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include "types.h"

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_type;

typedef struct {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 rbx;
    u64 rbp;
    u64 rsp;
} task_context_type;

typedef struct task_struct {
    u32 id;
    task_state_type state;
    task_context_type context;
    void *kernel_stack;
    const char *name;
    struct task_struct *next;
    u64 sleep_until;
} task_type;

extern task_type *current_task;
extern task_type *ready_queue;

void task_init(void);
task_type *task_create(void (*entry)(void *arg), void *arg, const char *name);
void task_exit(void);
void task_yield(void);
void task_block(void);
void task_wakeup(task_type *task);
void task_sleep_ms(u32 ms);
void reap_dead_tasks(void);
void schedule(void);

extern void task_switch(task_context_type *old, task_context_type *new);
extern void task_start(void);

#endif