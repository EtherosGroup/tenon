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
    ku64 r15;
    ku64 r14;
    ku64 r13;
    ku64 r12;
    ku64 rbx;
    ku64 rbp;
    ku64 rsp;
} task_context_type;

typedef struct task_struct {
    ku32 id;
    task_state_type state;
    task_context_type context;
    void *kernel_stack;
    const char *name;
    struct task_struct *next;
} task_type;

extern task_type *current_task;
extern task_type *ready_queue;

void task_init(void);
task_type *task_create(void (*entry)(void), const char *name);
void task_exit(void);
void task_yield(void);
void schedule(void);

extern void task_switch(task_context_type *old, task_context_type *new);
extern void task_start(void);

#endif