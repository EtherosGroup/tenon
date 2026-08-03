#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#include "types.h"

#define PIC1 0x20 // 主片命令端口
#define PIC1_DATA 0x21 // 主片数据端口
#define PIC2 0xA0 // 从片命令端口
#define PIC2_DATA 0xA1 // 从片数据端口

#define PIC_EOI 0x20 // 中断结束命令

/**
 * 重映射 PIC
 */
void pic_init(void);

/**
 * 发送 EOI（从片需同时发给主片）
 */
void pic_eoi(ku8 irq);

/**
 * 关闭IRQ线
 */
void pic_mask(ku8 irq);

/**
 * 开启IRQ线
 */
void pic_unmask(ku8 irq);

#endif