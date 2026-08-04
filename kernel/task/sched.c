#include "task.h"
#include "serial.h"
#include "asm.h"

void schedule(void)
{
    asm_cli();

    task_type *prev = current_task;

    task_type *next;
    if (prev->state == TASK_DEAD)
    {
        next = ready_queue;
    }
    else
    {
        next = prev->next;
    }

    task_type *start = next;
    do {
        if (next->state == TASK_READY)
        {
            break;
        }
        next = next->next;
    } while (next != start);

    if (next->state != TASK_READY) {
        reap_dead_tasks();
        asm_sti();
        return;
    }

    if (prev->state == TASK_RUNNING)
    {
        prev->state = TASK_READY;
    }

    next->state = TASK_RUNNING;
    current_task = next;

    task_switch(&prev->context, &next->context);

    asm_sti();
}