#include "block.h"

block_device_type *block_devices[BLOCK_MAX_DEVICES];
int block_device_count;

int block_register(block_device_type *dev)
{
    if (block_device_count >= BLOCK_MAX_DEVICES)
    {
        return -1;
    }
    block_devices[block_device_count++] = dev;
    return 0;
}

int block_read(block_device_type *dev, u64 lba, u32 count, void *buf)
{
    if (!dev || !dev->ops || !dev->ops->read)
    {
        return -1;
    }
    return dev->ops->read(dev, lba, count, buf);
}

int block_write(block_device_type *dev, u64 lba, u32 count, const void *buf)
{
    if (!dev || !dev->ops || !dev->ops->write)
    {
        return -1;
    }
    return dev->ops->write(dev, lba, count, buf);
}

int block_flush(block_device_type *dev)
{
    if (!dev || !dev->ops || !dev->ops->flush)
    {
        return -1;
    }
    return dev->ops->flush(dev);
}
