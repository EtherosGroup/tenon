#include "fb.h"
#include "vmm.h"
#include "memlayout.h"

u32 fb_width;
u32 fb_height;
u32 fb_pitch;
u8 fb_bpp;
u8 fb_r_shift;
u8 fb_g_shift;
u8 fb_b_shift;

static u32 *fb_vaddr;

void fb_init(u64 fb_pa, u32 width, u32 height, u32 pitch, u8 bpp, u8 r_pos, u8 r_size, u8 g_pos, u8 g_size, u8 b_pos, u8 b_size)
{
    (void)r_size;
    (void)g_size;
    (void)b_size;

    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
    fb_r_shift = r_pos;
    fb_g_shift = g_pos;
    fb_b_shift = b_pos;

    u64 fb_size = (u64)pitch * height;
    u64 pa_start = PAGE_ALIGN_DOWN(fb_pa);
    u64 pa_end   = PAGE_ALIGN_UP(fb_pa + fb_size);

    for (u64 pa = pa_start; pa < pa_end; pa += PAGE_SIZE)
    {
        vmm_map_page(&kernel_as, PHYS_TO_DIRECT(pa), pa, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
    }

    fb_vaddr = (u32 *)PHYS_TO_DIRECT(fb_pa);
}

void fb_put_pixel(u32 x, u32 y, u32 color)
{
    if (x >= fb_width || y >= fb_height)
    {
        return;
    }

    u32 offset = y * (fb_pitch / 4) + x;
    fb_vaddr[offset] = color;
}

u32 fb_get_pixel(u32 x, u32 y)
{
    if (x >= fb_width || y >= fb_height)
    {
        return 0;
    }
    u32 offset = y * (fb_pitch / 4) + x;
    return fb_vaddr[offset];
}