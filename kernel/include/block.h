#ifndef KERNEL_BLOCK_H
#define KERNEL_BLOCK_H

#include "types.h"

typedef struct block_device block_device_type;

struct block_device
{
    char name[32];
    u32 sector_size;
    u64 total_sectors;
    int (*read)(block_device_type *dev, u64 lba, u32 count, void *buf);
    int (*write)(block_device_type *dev, u64 lba, u32 count, const void *buf);
    void *priv;
};

int block_read(block_device_type *dev, u64 lba, u32 count, void *buf);
int block_write(block_device_type *dev, u64 lba, u32 count, const void *buf);

#endif