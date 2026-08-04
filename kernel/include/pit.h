#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include "types.h"

/**
 * 初始化并设置PIT
 */
void pit_init(ku32 freq_hz);

/**
 * 处理tick
 */
void pit_tick_handler(void);

/**
 * 等待
 */
void sleep_ms(ku32 ms);

#endif