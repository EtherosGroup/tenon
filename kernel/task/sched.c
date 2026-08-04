#include "task.h"
#include "serial.h"

void schedule(void)
{
    task_type *prev = current_task;

    task_type *start = prev->next;
    task_type *next = start;
    do {
        if (next->state == TASK_READY)
        {
            break;
        }
        next = next->next;
    } while (next != start);

    if (next->state != TASK_READY)
    {
        return;
    }

    if (prev->state == TASK_RUNNING)
    {
        prev->state = TASK_READY;
    }

    next->state = TASK_RUNNING;
    current_task = next;

    task_switch(&prev->context, &next->context);
}