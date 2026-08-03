#include "serial.h"
#include "asm.h"

#define COM1 0x3F8

static int is_transmit_empty(void)
{
    return asm_inb(COM1 + 5) & 0x20;
}

void kprint(char chars[])
{
    for (int i = 0; chars[i] != '\0'; i++)
    {
        while (!is_transmit_empty()) {}
        asm_outb(COM1, chars[i]);
    }
}

void kprintln(char chars[])
{
    kprint(chars);
    kprint("\n");
}
