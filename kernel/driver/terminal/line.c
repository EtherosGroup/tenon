#include "line.h"
#include "keyboard.h"
#include "terminal.h"

static void line_redraw(char *buf, u32 len, u32 pos)
{
    u32 saved = pos;
    for (u32 i = pos; i < len; i++)
        terminal_putchar(buf[i]);
    terminal_putchar(' ');
    u32 steps = len - saved + 1;
    for (u32 i = 0; i < steps; i++)
        terminal_cursor_left();
    terminal_draw_cursor();
}

void line_read(char *buf, u32 max_len)
{
    u32 len = 0;
    u32 pos = 0;

    for (;;)
    {
        char c = keyboard_readchar();

        if (c == '\n' || c == '\r')
        {
            while (pos < len) { terminal_cursor_right(); pos++; }
            terminal_putchar('\n');
            buf[len] = '\0';
            return;
        }

        if (c == '\b' && pos > 0)
        {
            pos--;
            for (u32 i = pos; i + 1 < len; i++)
                buf[i] = buf[i + 1];
            len--;
            terminal_cursor_left();
            line_redraw(buf, len, pos);
            continue;
        }

        if (c == KEY_DELETE && pos < len)
        {
            for (u32 i = pos; i + 1 < len; i++)
                buf[i] = buf[i + 1];
            len--;
            line_redraw(buf, len, pos);
            continue;
        }

        if (c == KEY_LEFT && pos > 0)
        {
            terminal_cursor_left();
            pos--;
            terminal_draw_cursor();
            continue;
        }

        if (c == KEY_RIGHT && pos < len)
        {
            terminal_cursor_right();
            pos++;
            terminal_draw_cursor();
            continue;
        }

        if (c == KEY_HOME && pos > 0)
        {
            while (pos > 0) { terminal_cursor_left(); pos--; }
            terminal_draw_cursor();
            continue;
        }

        if (c == KEY_END && pos < len)
        {
            while (pos < len) { terminal_cursor_right(); pos++; }
            terminal_draw_cursor();
            continue;
        }

        if (c >= 32 && c <= 126 && len < max_len - 1)
        {
            for (u32 i = len; i > pos; i--)
                buf[i] = buf[i - 1];
            buf[pos] = c;
            len++;
            pos++;
            terminal_putchar(c);
            line_redraw(buf, len, pos);
            continue;
        }

        terminal_draw_cursor();
    }
}
