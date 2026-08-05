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

static int ata_read_sectors(block_device_type *dev, u64 lba, u32 count, void *buf)
{
    if (count == 0)
    {
        return 0;
    }

    ata_channel_type *ch = (ata_channel_type *)dev->priv;

    if (ata_wait_bsy(ch) != 0)
    {
        return -1;
    }
    asm_outb(ch->ctrl_port, 0x00);

    ata_select_drive(ch, lba);

    // LBA48 high word
    asm_outb(ch->seccount_port, (u8)((count >> 8) & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)((lba >> 24) & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 32) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 40) & 0xFF));

    // LBA48 low word
    asm_outb(ch->seccount_port, (u8)(count & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)(lba & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 8) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 16) & 0xFF));

    asm_outb(ch->command_port, ATA_CMD_READ_PIO_EXT);

    u16 *dst = (u16 *)buf;

    for (u32 sector = 0; sector < count; sector++)
    {
        if (ata_wait_drq(ch) != 0)
        {
            return -1;
        }

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
    if (count == 0)
    {
        return 0;
    }

    ata_channel_type *ch = (ata_channel_type *)dev->priv;

    if (ata_wait_bsy(ch) != 0)
    {
        return -1;
    }
    asm_outb(ch->ctrl_port, 0x00);

    ata_select_drive(ch, lba);

    // LBA48 high word
    asm_outb(ch->seccount_port, (u8)((count >> 8) & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)((lba >> 24) & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 32) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 40) & 0xFF));

    // LBA48 low word
    asm_outb(ch->seccount_port, (u8)(count & 0xFF));
    asm_outb(ch->lba_lo_port, (u8)(lba & 0xFF));
    asm_outb(ch->lba_mid_port, (u8)((lba >> 8) & 0xFF));
    asm_outb(ch->lba_hi_port, (u8)((lba >> 16) & 0xFF));

    asm_outb(ch->command_port, ATA_CMD_WRITE_PIO_EXT);

    const u16 *src = (const u16 *)buf;

    for (u32 sector = 0; sector < count; sector++)
    {
        if (ata_wait_drq(ch) != 0)
        {
            return -1;
        }

        for (int i = 0; i < 256; i++)
        {
            asm_outw(ch->data_port, src[i]);
        }

        src += 256;
        asm_io_wait();
    }

    if (ata_wait_bsy(ch) != 0)
    {
        return -2;
    }

    return (int)count;
}

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

    // Select drive
    asm_outb(ch->drive_port, drive_select);
    asm_io_wait();
    asm_io_wait();

    serial_print("[ATA] Probing ");
    serial_print(name);
    serial_print("... ");

    // Check presence
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

    // Send IDENTIFY
    asm_outb(ch->drive_port, drive_select);
    asm_io_wait();
    asm_outb(ch->seccount_port, 0);
    asm_outb(ch->lba_lo_port, 0);
    asm_outb(ch->lba_mid_port, 0);
    asm_outb(ch->lba_hi_port, 0);
    asm_outb(ch->command_port, ATA_CMD_IDENTIFY);

    u8 sr = asm_inb(ch->status_port);
    if (sr == 0x00)
    {
        serial_println("no device");
        return null;
    }

    if (ata_wait_bsy(ch) != 0)
    {
        serial_println("timeout");
        return null;
    }

    // Check for ATAPI
    u8 mid = asm_inb(ch->lba_mid_port);
    u8 hi  = asm_inb(ch->lba_hi_port);
    if (mid == 0x14 && hi == 0xEB)
    {
        serial_println("ATAPI (skipped)");
        return null;
    }

    if (ata_wait_drq(ch) != 0)
    {
        serial_println("no DRQ");
        return null;
    }

    // Read IDENTIFY data (256 words)
    u16 id[256];
    for (int i = 0; i < 256; i++)
    {
        id[i] = asm_inw(ch->data_port);
    }

    // LBA48 sectors: words 100-103
    u64 total_sectors = (u64)id[100]
    | ((u64)id[101] << 16)
    | ((u64)id[102] << 32)
    | ((u64)id[103] << 48);

    // Fall back to LBA28: words 60-61
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
        serial_println("[ATA] Failed to allocate block_device_t");
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
    dev->read = ata_read_sectors;
    dev->write = ata_write_sectors;
    dev->priv = ch;
    channel_count++;

    return dev;
}