#include "kernel.h"

void start_kernel()
{
    kprint("\n");
    kprintln("aaaaaa");

    idt_init();
    pic_init();
}