#include "idt.h"
#include "pic.h"
#include "serial.h"
#include "asm.h"
#include "keyboard.h"
#include "pit.h"
#include "task.h"

static void hex_print(u8 byte)
{
    static const char hex[] = "0123456789ABCDEF";
    serial_print("0x");
    char buf[3];
    buf[0] = hex[byte >> 4];
    buf[1] = hex[byte & 0xF];
    buf[2] = '\0';
    serial_print(buf);
}

static void exception_handler(int_frame_type *frame)
{
    serial_print("Exception #");
    hex_print((u8)frame->vector);
    serial_print(" err=");
    serial_println("");
    asm_halt();
}

void isr_handler(int_frame_type *frame)
{
    if (frame->vector < 32)
    {
        exception_handler(frame);
    }
    else if (frame->vector >= IRQ_BASE)
    {
        u8 irq = frame->vector - IRQ_BASE;
        if (irq == 0)
        {
            pic_eoi(0);
            pit_tick_handler();

            task_type *t = ready_queue;
            do
            {
                if (t->state == TASK_BLOCKED && t->sleep_until != 0
                        && tick_count >= t->sleep_until)
                {
                    t->state = TASK_READY;
                    t->sleep_until = 0;
                }
                t = t->next;
            }
            while (t != ready_queue);

            if (tick_count % 10 == 0)
            {
                schedule();
            }
        }
        else if (irq == 1)
        {
            pic_eoi(1);
            keyboard_handler();
        }
        else
        {
            pic_eoi(irq);
        }
    }
}