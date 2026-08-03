#include "idt.h"
#include "pic.h"
#include "serial.h"
#include "asm.h"
#include "keyboard.h"

static void hex_print(ku8 byte) {
    static const char hex[] = "0123456789ABCDEF";
    kprint("0x");
    char buf[3];
    buf[0] = hex[byte >> 4];
    buf[1] = hex[byte & 0xF];
    buf[2] = '\0';
    kprint(buf);

}

static void exception_handler(int_frame_type *frame)
{
    kprint("Exception #");
    hex_print((ku8)frame->vector);
    kprint(" err=");
    // 需实现 hex64 打印 error_code... 先简化为只打 vector
    kprintln("");
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
        ku8 irq = frame->vector - IRQ_BASE;
        if (irq == 1)
        // 键盘
        {
            keyboard_handler();
        }
        pic_eoi(irq);
    }
}