#ifndef KERNEL_FB_H
#define KERNEL_FB_H

#include "types.h"

extern u32 fb_width;
extern u32 fb_height;
extern u32 fb_pitch;
extern u8  fb_bpp;
extern u8  fb_r_shift;
extern u8  fb_g_shift;
extern u8  fb_b_shift;

void fb_init(u64 fb_pa, u32 width, u32 height, u32 pitch, u8 bpp, u8 r_pos, u8 r_size, u8 g_pos, u8 g_size, u8 b_pos, u8 b_size);

void fb_put_pixel(u32 x, u32 y, u32 color);
u32 fb_get_pixel(u32 x, u32 y);

#define RGB(r, g, b) (((u32)(b) << fb_b_shift) | ((u32)(g) << fb_g_shift) | ((u32)(r) << fb_r_shift))

#define COLOR_BLACK RGB(0, 0, 0)
#define COLOR_BLUE RGB(0, 0, 170)
#define COLOR_GREEN RGB(0, 170, 0)
#define COLOR_CYAN RGB(0, 170, 170)
#define COLOR_RED RGB(170, 0, 0)
#define COLOR_MAGENTA RGB(170, 0, 170)
#define COLOR_BROWN RGB(170, 85, 0)
#define COLOR_LIGHT_GREY RGB(170, 170, 170)
#define COLOR_DARK_GREY RGB(85, 85, 85)
#define COLOR_LIGHT_BLUE RGB(85, 85, 255)
#define COLOR_LIGHT_GREEN RGB(85, 255, 85)
#define COLOR_LIGHT_CYAN RGB(85, 255, 255)
#define COLOR_LIGHT_RED RGB(255, 85, 85)
#define COLOR_LIGHT_MAGENTA RGB(255, 85, 255)
#define COLOR_YELLOW RGB(255, 255, 85)
#define COLOR_WHITE RGB(255, 255, 255)

#endif