#ifndef KERNEL_BLOCK_H
#define KERNEL_BLOCK_H

#include "types.h"

typedef enum
{
    BLOCK_TRANSPORT_ATA  = 0,
    BLOCK_TRANSPORT_AHCI = 1,
    BLOCK_TRANSPORT_NVME = 2,
} block_transport_t;

#define BLOCK_FEAT_LBA48 (1 << 0)
#define BLOCK_FEAT_DMA (1 << 1)
#define BLOCK_FEAT_TRIM (1 << 2)
#define BLOCK_FEAT_FLUSH (1 << 3)

typedef struct block_device block_device_type;

typedef struct
{
    char model[41];
    char serial[21];
    u32 max_sectors_per_io;
    u32 features;
} block_dev_info_type;

typedef struct
{
    int (*read)(block_device_type *dev, u64 lba, u32 count, void *buf);
    int (*write)(block_device_type *dev, u64 lba, u32 count, const void *buf);
    int (*flush)(block_device_type *dev);
    int (*identify)(block_device_type *dev, block_dev_info_type *info);
} block_device_ops_type;

struct block_device
{
    char name[32];
    u32 sector_size;
    u64 total_sectors;
    block_transport_t   transport;
    block_device_ops_type const *ops;
    block_dev_info_type info;
    void *driver_data;
};

#define BLOCK_MAX_DEVICES 8

extern block_device_type *block_devices[BLOCK_MAX_DEVICES];
extern int block_device_count;

int block_register(block_device_type *dev);

int block_read(block_device_type *dev, u64 lba, u32 count, void *buf);
int block_write(block_device_type *dev, u64 lba, u32 count, const void *buf);
int block_flush(block_device_type *dev);

#endif
