#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include "types.h"

#define MULTIBOOT2_MAGIC 0x36d76289
#define MULTIBOOT_TAG_END 0
#define MULTIBOOT_TAG_MMAP 6
#define MULTIBOOT_TAG_FRAMEBUFFER 8
#define MULTIBOOT_MEMORY_AVAILABLE 1

typedef struct
{
    u32 total_size;
    u32 reserved;
} multiboot_info_type;

typedef struct
{
    u32 type;
    u32 size;
} multiboot_tag_type;

typedef struct
{
    u32 type;
    u32 size;
} __attribute__((packed)) multiboot_tag_header_type;

typedef struct
{
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
} multiboot_tag_mmap_type;

typedef struct
{
    u64 base_addr;
    u64 length;
    u32 type;
    u32 reserved;
} multiboot_mmap_entry_type;

typedef struct
{
    u32 type;
    u32 size;
    u64 fb_addr;
    u32 fb_pitch;
    u32 fb_width;
    u32 fb_height;
    u8 fb_bpp;
    u8 fb_type;
    u16 reserved;
    u8 fb_red_field_position;
    u8 fb_red_mask_size;
    u8 fb_green_field_position;
    u8 fb_green_mask_size;
    u8 fb_blue_field_position;
    u8 fb_blue_mask_size;
} __attribute__((packed)) multiboot_tag_fb_type;

#endif