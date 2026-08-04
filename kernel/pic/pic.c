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

void pic_eoi(u8 irq)
{
    if (irq >= 8)
    // irq >= 8 说明是从片发的，此时先发从片后再发主片
    {
        asm_outb(PIC2, PIC_EOI);
    }
    asm_outb(PIC1, PIC_EOI); // 主片
}

/**
 * IMR (中断屏蔽寄存器) 中每一位对应一个中断源：
 *   1 = 该中断被屏蔽 (禁止响应)
 *   0 = 该中断未被屏蔽 (允许响应)
 *
 * 此函数只修改目标 IRQ 对应的位，其他位保持不变。
 * 例如：操作 IRQ3，从 0b11101111 -> 0b11100111 (只改变 bit3)
 *
 * 注意：
 *   - IRQ 0~7 使用主 PIC (端口 0x21)
 *   - IRQ 8~15 使用从 PIC (端口 0xA1)
 *   - 使用 irq & 7 将中断号映射到 PIC 内部的 0~7 位
 */


void pic_mask(u8 irq)
{
    u16 port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    asm_outb(port, asm_inb(port) | (1 << (irq & 7)));
}

void pic_unmask(u8 irq)
{
    u16 port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    asm_outb(port, asm_inb(port) & ~(1 << (irq & 7)));
}
