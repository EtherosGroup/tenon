#include "ata.h"
#include "asm.h"
#include "serial.h"
#include "pit.h"
#include "kheap.h"

typedef struct
{
    u16 data_port;
    u16 seccount_port;
    u16 lba_lo_port;
    u16 lba_mid_port;
    u16 lba_hi_port;
    u16 drive_port;
    u16 status_port;
    u16 command_port;
    u16 ctrl_port;
    u8 drive_select;
} ata_channel_type;

static ata_channel_type channels[4];
static int channel_count;

static int ata_wait_bsy(ata_channel_type *ch)
{
    int timeout = 100000;
    while (timeout--)
    {
        u8 status = asm_inb(ch->status_port);
        if (!(status & ATA_SR_BSY))
        {
            return 0;
        }
        asm_io_wait();
    }
    return -1;
}

static int ata_wait_drq(ata_channel_type *ch)
{
    int timeout = 100000;
    while (timeout--)
    {
        u8 status = asm_inb(ch->status_port);
        if (status & ATA_SR_ERR)
        {
            return -1;
        }
        if (status & ATA_SR_DRQ)
        {
            return 0;
        }
        asm_io_wait();
    }
    return -1;
}

static int ata_wait_ready(ata_channel_type *ch)
{
    int timeout = 200000;
    while (timeout--)
    {
        u8 status = asm_inb(ch->status_port);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY))
        {
            return 0;
        }
        asm_io_wait();
    }
    return -1;
}

static void ata_select_drive(ata_channel_type *ch, u64 lba)
{
    u8 drive_byte = ch->drive_select | ((lba >> 24) & 0x0F);
    asm_outb(ch->drive_port, drive_byte);
}

static void ata_parse_string(u16 *id, int start, int len, char *out)
{
    for (int i = 0; i < len / 2; i++)
    {
        u16 w = id[start + i];
        out[i * 2]     = (char)(w >> 8);
        out[i * 2 + 1] = (char)(w & 0xFF);
    }
    out[len] = '\0';
    for (int i = len - 1; i >= 0; i--)
    {
        if (out[i] == ' ')
        {
            out[i] = '\0';
        }
        else
        {
            break;
        }
    }
}

static int ata_do_identify(ata_channel_type *ch, u16 *id)
{
    asm_outb(ch->drive_port, ch->drive_select);
    asm_io_wait();
    asm_outb(ch->seccount_port, 0);
    asm_outb(ch->lba_lo_port, 0);
    asm_outb(ch->lba_mid_port, 0);
    asm_outb(ch->lba_hi_port, 0);
    asm_outb(ch->command_port, ATA_CMD_IDENTIFY);

    u8 sr = asm_inb(ch->status_port);
    if (sr == 0x00) return -1;

    if (ata_wait_bsy(ch) != 0) return -1;

    u8 mid = asm_inb(ch->lba_mid_port);
    u8 hi = asm_inb(ch->lba_hi_port);
    if (mid == 0x14 && hi == 0xEB) return -1;

    if (ata_wait_drq(ch) != 0) return -1;

    for (int i = 0; i < 256; i++)
    {
        id[i] = asm_inw(ch->data_port);
    }
    return 0;
}

static void ata_fill_info(u16 *id, block_dev_info_type *info)
{
    ata_parse_string(id, 27, 40, info->model);
    ata_parse_string(id, 10, 20, info->serial);

    info->max_sectors_per_io = id[47] & 0xFF;
    if (info->max_sectors_per_io == 0)
    {
        info->max_sectors_per_io = 1;
    }

    info->features = BLOCK_FEAT_LBA48;
}

static int ata_read_sectors(block_device_type *dev, u64 lba, u32 count, void *buf)
{
    if (count == 0) return 0;

    ata_channel_type *ch = (ata_channel_type *)dev->driver_data;

    if (ata_wait_bsy(ch) != 0) return -1;
    asm_outb(ch->ctrl_port, 0x00);

    ata_select_drive(ch, lba);

    asm_outb(ch->seccount_port, (u8)((count >> 8) & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)((lba >> 24) & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 32) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 40) & 0xFF));

    asm_outb(ch->seccount_port, (u8)(count & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)(lba & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 8) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 16) & 0xFF));

    asm_outb(ch->command_port, ATA_CMD_READ_PIO_EXT);

    u16 *dst = (u16 *)buf;

    for (u32 sector = 0; sector < count; sector++)
    {
        if (ata_wait_drq(ch) != 0) return -1;

        for (int i = 0; i < 256; i++)
        {
            dst[i] = asm_inw(ch->data_port);
        }

        dst += 256;
        asm_io_wait();
    }

    return (int)count;
}

static int ata_write_sectors(block_device_type *dev, u64 lba, u32 count, const void *buf)
{
    if (count == 0) return 0;

    ata_channel_type *ch = (ata_channel_type *)dev->driver_data;

    if (ata_wait_bsy(ch) != 0) return -1;
    asm_outb(ch->ctrl_port, 0x00);

    ata_select_drive(ch, lba);

    asm_outb(ch->seccount_port, (u8)((count >> 8) & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)((lba >> 24) & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 32) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 40) & 0xFF));

    asm_outb(ch->seccount_port, (u8)(count & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)(lba & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 8) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 16) & 0xFF));

    asm_outb(ch->command_port, ATA_CMD_WRITE_PIO_EXT);

    const u16 *src = (const u16 *)buf;

    for (u32 sector = 0; sector < count; sector++)
    {
        if (ata_wait_drq(ch) != 0) return -1;

        for (int i = 0; i < 256; i++)
        {
            asm_outw(ch->data_port, src[i]);
        }

        src += 256;
        asm_io_wait();
    }

    if (ata_wait_bsy(ch) != 0) return -2;

    return (int)count;
}

static int ata_flush(block_device_type *dev)
{
    (void)dev;
    return 0;
}

static int ata_identify(block_device_type *dev, block_dev_info_type *info)
{
    ata_channel_type *ch = (ata_channel_type *)dev->driver_data;
    u16 id[256];

    if (ata_do_identify(ch, id) != 0) return -1;

    ata_fill_info(id, info);

    ata_parse_string(id, 27, 40, dev->info.model);
    ata_parse_string(id, 10, 20, dev->info.serial);
    dev->info.max_sectors_per_io = info->max_sectors_per_io;
    dev->info.features = info->features;

    return 0;
}

static const block_device_ops_type ata_ops = {
    .read = ata_read_sectors,
    .write = ata_write_sectors,
    .flush = ata_flush,
    .identify = ata_identify,
};

block_device_type *ata_init(u16 base, u16 ctrl, u8 drive_select, const char *name)
{
    if (channel_count >= 4)
    {
        serial_println("[ATA] No channel slots left");
        return null;
    }

    ata_channel_type *ch = &channels[channel_count];

    ch->data_port = base;
    ch->seccount_port = base + 2;
    ch->lba_lo_port = base + 3;
    ch->lba_mid_port = base + 4;
    ch->lba_hi_port = base + 5;
    ch->drive_port = base + 6;
    ch->status_port = base + 7;
    ch->command_port = base + 7;
    ch->ctrl_port = ctrl;
    ch->drive_select = drive_select;

    asm_outb(ch->drive_port, drive_select);
    asm_io_wait();
    asm_io_wait();

    serial_print("[ATA] Probing ");
    serial_print(name);
    serial_print("... ");

    if (ata_wait_ready(ch) != 0)
    {
        serial_println("not present");
        return null;
    }

    u8 status = asm_inb(ch->status_port);
    if (status == 0xFF || status == 0x00)
    {
        serial_println("no device");
        return null;
    }

    u16 id[256];
    if (ata_do_identify(ch, id) != 0)
    {
        serial_println("ATAPI or timeout");
        return null;
    }

    u64 total_sectors = (u64)id[100]
    | ((u64)id[101] << 16)
    | ((u64)id[102] << 32)
    | ((u64)id[103] << 48);

    if (total_sectors == 0)
    {
        total_sectors = (u64)id[60] | ((u64)id[61] << 16);
    }

    serial_print("found, ");
    serial_print_dec(total_sectors);
    serial_println(" sectors");

    block_device_type *dev = (block_device_type *)kmalloc(sizeof(block_device_type));
    if (!dev)
    {
        serial_println("[ATA] Failed to allocate block_device_type");
        return null;
    }

    int i = 0;
    while (name[i] && i < 31)
    {
        dev->name[i] = name[i];
        i++;
    }
    dev->name[i] = '\0';

    dev->sector_size = ATA_SECTOR_SIZE;
    dev->total_sectors = total_sectors;
    dev->transport = BLOCK_TRANSPORT_ATA;
    dev->ops = &ata_ops;
    dev->driver_data = ch;

    ata_fill_info(id, &dev->info);

    serial_print("  model: ");
    serial_println(dev->info.model);
    serial_print("  serial: ");
    serial_println(dev->info.serial);

    channel_count++;
    block_register(dev);

    return dev;
}
