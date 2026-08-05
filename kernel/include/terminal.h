#ifndef KERNEL_TERMINAL_H
#define KERNEL_TERMINAL_H

#include "types.h"

void terminal_init(u32 width, u32 height, u32 fg, u32 bg);
void terminal_putchar(char c);
void terminal_write(const char *s);
void terminal_setcolor(u32 fg, u32 bg);
void terminal_clear(void);
void terminal_draw_cursor(void);
void terminal_cursor_left(void);
void terminal_cursor_right(void);

#endif