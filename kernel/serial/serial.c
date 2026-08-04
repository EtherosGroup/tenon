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

static void kputc(char c)
{
    while (!is_transmit_empty()) {}
    asm_outb(COM1, c);
}

void kprint_hex(ku64 num)
{
    kprint("0x");
    if (num == 0) {
        kputc('0');
        return;
    }
    char buf[16];
    int i = 0;
    while (num) {
        ku8 d = num & 0xF;
        buf[i++] = d < 10 ? '0' + d : 'A' + d - 10;
        num >>= 4;
    }
    while (i--) kputc(buf[i]);
}

void kprint_dec(ku64 num)
{
    if (num == 0) {
        kputc('0');
        return;
    }
    char buf[20];
    int i = 0;
    while (num) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i--) kputc(buf[i]);
}
