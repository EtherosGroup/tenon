#include "pit.h"
#include "asm.h"

// 系统启动以来的总 tick 数
volatile u64 tick_count = 0;
static volatile u64 ticks;

void pit_init(u32 freq_hz)
{
    // 计算分频值
    u32 divisor = 1193182 / freq_hz;
    // 命令：通道0、模式3(方波)、先低后高
    asm_outb(0x43, 0x36);
    // 低 8 位
    asm_outb(0x40, divisor & 0xFF);
    // 高 8 位
    asm_outb(0x40, (divisor >> 8) & 0xFF);
}

void pit_tick_handler(void)
{
    tick_count++;
}

void sleep_ms(u32 ms)
{
    u64 target = tick_count + ms; // tick_count 每 ms 加 1 (1000Hz 时)
    while (tick_count < target)
    {
        asm_hlt(); // 等中断唤醒
    }
}