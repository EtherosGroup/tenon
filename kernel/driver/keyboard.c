#include "asm.h"
#include "keyboard.h"

#define KEYBOARD_DATA 0x60

static const char sc_ascii[128] =
{
    0,   0,   '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*',
    0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   '-', 0,   0,   0,   '+', 0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
};

static char ring_buf[KEYBOARD_RING_SIZE];
static volatile u32 ring_head;
static volatile u32 ring_tail;
static bool e0_prefix;

void keyboard_handler(void)
{
    u8 sc = asm_inb(KEYBOARD_DATA);

    if (sc == 0xE0)
    {
        e0_prefix = true;
        return;
    }

    if (sc & 0x80)
    {
        e0_prefix = false;
        return;
    }

    if (e0_prefix)
    {
        e0_prefix = false;
        if (sc == 0x4B) keyboard_ring_push(KEY_LEFT);
        if (sc == 0x4D) keyboard_ring_push(KEY_RIGHT);
        if (sc == 0x53) keyboard_ring_push(KEY_DELETE);
        if (sc == 0x47) keyboard_ring_push(KEY_HOME);
        if (sc == 0x4F) keyboard_ring_push(KEY_END);
        return;
    }

    char ch = sc_ascii[sc];
    if (ch)
    {
        keyboard_ring_push(ch);
    }
}

void keyboard_ring_init(void)
{
    ring_head = 0;
    ring_tail = 0;
}

void keyboard_ring_push(char c)
{
    u32 next = (ring_head + 1) % KEYBOARD_RING_SIZE;
    if (next == ring_tail)
    {
        return;
    }
    ring_buf[ring_head] = c;
    ring_head = next;
}

char keyboard_ring_pop(void)
{
    char c = ring_buf[ring_tail];
    ring_tail = (ring_tail + 1) % KEYBOARD_RING_SIZE;
    return c;
}

bool keyboard_ring_empty(void)
{
    return ring_head == ring_tail;
}

char keyboard_readchar(void)
{
    while (keyboard_ring_empty())
    {
        asm_hlt();
    }
    return keyboard_ring_pop();
}
