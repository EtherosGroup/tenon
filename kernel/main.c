#include "kernel.h"

void start_kernel()
{
    kprint("\n");
    kprintln("Tenon v0.0.1");

    idt_init();
    pic_init();
    pic_unmask(1); // 开键盘（IRQ1）
    asm_sti(); // 开CPU中断

    for (;;) {
        asm_hlt();
    }
}