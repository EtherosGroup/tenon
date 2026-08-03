#include "pic.h"
#include "asm.h"

void pic_init(void)
{
    // ICW1: 开始初始化，边沿触发，级联，需要 ICW4
    asm_outb(PIC1, 0x11);
    asm_outb(PIC2, 0x11);

    // ICW2: 向量偏移
    asm_outb(PIC1_DATA, 0x20); // 主片 IRQ 0-7 → 向量 0x20-0x27
    asm_outb(PIC2_DATA, 0x28); // 从片 IRQ 8-15 → 向量 0x28-0x2F

    // ICW3: 级联连接
    asm_outb(PIC1_DATA, 0x04); // 主片 IRQ2 连从片
    asm_outb(PIC2_DATA, 0x02); // 从片标识号 = 2

    // ICW4: 8086 模式
    asm_outb(PIC1_DATA, 0x01);
    asm_outb(PIC2_DATA, 0x01);

    // 屏蔽所有中断，后边按需打开
    asm_outb(PIC1_DATA, 0xFF);
    asm_outb(PIC2_DATA, 0xFF);
}

void pic_eoi(ku8 irq)
{
    if (irq >= 8)
    // irq >= 8 说明是从片发的，此时先发从片后再发主片
    {
        asm_outb(PIC2, PIC_EOI);
    }
    asm_outb(PIC1, PIC_EOI); // 主片
}

void pic_mask(ku8 irq)
{

}

void pic_unmask(ku8 irq)
{
    
}
