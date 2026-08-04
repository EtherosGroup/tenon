#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include "types.h"

#define MULTIBOOT2_MAGIC 0x36d76289 // m2魔数
#define MULTIBOOT_TAG_END 0
#define MULTIBOOT_TAG_MMAP 6
#define MULTIBOOT_MEMORY_AVAILABLE 1

typedef struct {
    ku32 total_size;
    ku32 reserved;
} multiboot_info_type;

typedef struct {
    ku32 type;
    ku32 size;
} multiboot_tag_type;

typedef struct {
    ku32 type;
    ku32 size;
    ku32 entry_size;
    ku32 entry_version;
} multiboot_tag_mmap_type;

typedef struct {
    ku64 base_addr;
    ku64 length;
    ku32 type;
    ku32 reserved;
} multiboot_mmap_entry_type;

#endif