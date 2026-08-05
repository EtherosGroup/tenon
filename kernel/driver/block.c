#include "block.h"

int block_read(block_device_type *dev, u64 lba, u32 count, void *buf)
{
    if (!dev || !dev->read)
    {
        return -1;
    }
    return dev->read(dev, lba, count, buf);
}

int block_write(block_device_type *dev, u64 lba, u32 count, const void *buf)
{
    if (!dev || !dev->write)
    {
        return -1;
    }
    return dev->write(dev, lba, count, buf);
}