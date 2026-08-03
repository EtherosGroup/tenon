#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

/**
 * 向QEMU串口输出字符串
 * @param chars[] 字符串
 */
void kprint(char chars[]);

/**
 * 向QEMU串口输出字符串，自动换行
 * @param chars[] 字符串
 */
void kprintln(char chars[]);

#endif
