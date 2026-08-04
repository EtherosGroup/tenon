#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include "types.h"

#define MULTIBOOT2_MAGIC 0x36d76289 // m2魔数
#define MULTIBOOT_TAG_END 0
#define MULTIBOOT_TAG_MMAP 6
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

#endif