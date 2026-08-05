#include "terminal.h"
#include "fb.h"
#include "font.h"

#define TAB_WIDTH 8

static u32 term_cols;
static u32 term_rows;
static u32 cursor_x;
static u32 cursor_y;
static u32 term_fg;
static u32 term_bg;

static void terminal_draw_char(char c, u32 x, u32 y, u32 fg, u32 bg)
{
    u32 px = x * FONT_WIDTH;
    u32 py = y * FONT_HEIGHT;

    if (c < 32 || c > 126)
    {
        c = ' ';
    }
    u32 idx = (u32)c - 32;

    for (u32 row = 0; row < FONT_HEIGHT; row++)
    {
        u8 bits = font_data[idx][row];
        for (u32 col = 0; col < FONT_WIDTH; col++)
        {
            u32 color = (bits & (0x80 >> col)) ? fg : bg;
            fb_put_pixel(px + col, py + row, color);
        }
    }
}

static void terminal_scroll(void)
{
    u32 glyph_height = FONT_HEIGHT;
    u32 glyph_width  = FONT_WIDTH;

    for (u32 y = 0; y < term_rows - 1; y++)
    {
        for (u32 x = 0; x < term_cols; x++)
        {
            u32 dst_px = x * glyph_width;
            u32 dst_py = y * glyph_height;
            u32 src_px = dst_px;
            u32 src_py = dst_py + glyph_height;

            for (u32 row = 0; row < glyph_height; row++)
            {
                for (u32 col = 0; col < glyph_width; col++)
                {
                    u32 color = fb_get_pixel(src_px + col, src_py + row);
                    fb_put_pixel(dst_px + col, dst_py + row, color);
                }
            }
        }
    }

    for (u32 x = 0; x < term_cols; x++)
    {
        terminal_draw_char(' ', x, term_rows - 1, term_bg, term_bg);
    }
}

void terminal_init(u32 width, u32 height, u32 fg, u32 bg)
{
    term_cols = width  / FONT_WIDTH;
    term_rows = height / FONT_HEIGHT;
    term_fg = fg;
    term_bg = bg;
    cursor_x = 0;
    cursor_y = 0;

    for (u32 y = 0; y < term_rows; y++)
    {
        for (u32 x = 0; x < term_cols; x++)
        {
            terminal_draw_char(' ', x, y, bg, bg);
        }
    }
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else if (c == '\r')
    {
        cursor_x = 0;
    }
    else if (c == '\b')
    {
        if (cursor_x > 0)
        {
            u32 px = cursor_x * FONT_WIDTH;
            u32 py = cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1;
            for (u32 col = 0; col < FONT_WIDTH; col++)
                fb_put_pixel(px + col, py, term_bg);

            cursor_x--;
            terminal_draw_char(' ', cursor_x, cursor_y, term_bg, term_bg);
        }
    }
    else if (c == '\t')
    {
        u32 spaces = TAB_WIDTH - (cursor_x % TAB_WIDTH);
        for (u32 i = 0; i < spaces; i++)
        {
            terminal_putchar(' ');
        }
        return;
    }
    else
    {
        terminal_draw_char(c, cursor_x, cursor_y, term_fg, term_bg);
        cursor_x++;
    }

    if (cursor_x >= term_cols)
    {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= term_rows)
    {
        terminal_scroll();
        cursor_y = term_rows - 1;
    }
}

void terminal_write(const char *s)
{
    for (u32 i = 0; s[i] != '\0'; i++)
    {
        terminal_putchar(s[i]);
    }
}

void terminal_setcolor(u32 fg, u32 bg)
{
    term_fg = fg;
    term_bg = bg;
}

void terminal_clear(void)
{
    cursor_x = 0;
    cursor_y = 0;

    for (u32 y = 0; y < term_rows; y++)
    {
        for (u32 x = 0; x < term_cols; x++)
        {
            terminal_draw_char(' ', x, y, term_bg, term_bg);
        }
    }
}

void terminal_draw_cursor(void)
{
    u32 px = cursor_x * FONT_WIDTH;
    u32 py = cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1;
    for (u32 col = 0; col < FONT_WIDTH; col++)
    {
        fb_put_pixel(px + col, py, term_fg);
    }
}

void terminal_cursor_left(void)
{
    if (cursor_x == 0) return;
    u32 px = cursor_x * FONT_WIDTH;
    u32 py = cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1;
    for (u32 col = 0; col < FONT_WIDTH; col++)
        fb_put_pixel(px + col, py, term_bg);
    cursor_x--;
}

void terminal_cursor_right(void)
{
    if (cursor_x + 1 >= term_cols) return;
    u32 px = cursor_x * FONT_WIDTH;
    u32 py = cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1;
    for (u32 col = 0; col < FONT_WIDTH; col++)
        fb_put_pixel(px + col, py, term_bg);
    cursor_x++;
}