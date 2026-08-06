#include "pci.h"
#include "asm.h"
#include "serial.h"

static u32 pci_make_addr(u8 bus, u8 device, u8 func, u8 offset)
{
    return (1U << 31) 
    | ((u32)bus << 16)
    | ((u32)(device & 0x1F) << 11)
    | ((u32)(func & 0x07) << 8)
    | ((u32)(offset & 0xFC));
}

u32 pci_config_read(u8 bus, u8 device, u8 func, u8 offset)
{
    u32 addr = pci_make_addr(bus, device, func, offset);
    asm_outl(PCI_CONFIG_ADDR, addr);
    return asm_inl(PCI_CONFIG_DATA);
}

u16 pci_config_read_word(u8 bus, u8 device, u8 func, u8 offset)
{
    u32 val = pci_config_read(bus, device, func, offset & 0xFC);
    return (u16)((val >> ((offset & 2) * 8)) & 0xFFFF);
}

static void pci_read_device(u8 bus, u8 device, u8 func, pci_device_type *out)
{
    out->bus = bus;
    out->device = device;
    out->func = func;
    out->vendor_id = pci_config_read_word(bus, device, func, PCI_VENDOR_ID);
    out->device_id = pci_config_read_word(bus, device, func, PCI_DEVICE_ID);
    out->class_code = (u8)(pci_config_read(bus, device, func, PCI_CLASS) >> 24);
    out->subclass = (u8)((pci_config_read(bus, device, func, PCI_CLASS) >> 16) & 0xFF);
    out->prog_if = (u8)((pci_config_read(bus, device, func, PCI_CLASS) >> 8) & 0xFF);
    out->header_type = (u8)((pci_config_read(bus, device, func, PCI_HEADER_TYPE) >> 16) & 0xFF);
    out->int_line = (u8)(pci_config_read(bus, device, func, PCI_INT_LINE) & 0xFF);
    out->int_pin = (u8)((pci_config_read(bus, device, func, PCI_INT_LINE) >> 8) & 0xFF);

    for (int i = 0; i < 6; i++)
    {
        out->bar[i] = pci_config_read(bus, device, func, PCI_BAR0 + i * 4);
    }
}

void pci_scan(void)
{
    serial_println("[PCI] Scanning bus...");

    for (u16 bus = 0; bus < PCI_MAX_BUS; bus++)
    {
        for (u8 device = 0; device < PCI_MAX_DEVICE; device++)
        {
            u8 header_type = pci_config_read_word(bus, device, 0, PCI_HEADER_TYPE);

            u8 max_func = 1;
            if (header_type & 0x80)
            {
                max_func = PCI_MAX_FUNC;
            }

            for (u8 func = 0; func < max_func; func++)
            {
                u16 vendor = pci_config_read_word(bus, device, func, PCI_VENDOR_ID);
                if (vendor == 0xFFFF)
                {
                    continue;
                }

                pci_device_type dev;
                pci_read_device(bus, device, func, &dev);

                serial_print("  ");
                serial_print_hex((u64)bus);
                serial_print(":");
                serial_print_hex((u64)device);
                serial_print(":");
                serial_print_hex((u64)func);
                serial_print(" vid=0x");
                serial_print_hex(dev.vendor_id);
                serial_print(" did=0x");
                serial_print_hex(dev.device_id);
                serial_print(" class=");
                serial_print_hex(dev.class_code);
                serial_print(".");
                serial_print_hex(dev.subclass);
                serial_println("");
            }
        }
    }

    serial_println("[PCI] Done.");
}

void pci_scan_class(u8 class_code, u8 subclass, pci_device_callback_type cb, void *ctx)
{
    for (u16 bus = 0; bus < PCI_MAX_BUS; bus++)
    {
        for (u8 device = 0; device < PCI_MAX_DEVICE; device++)
        {
            u8 header_type = pci_config_read_word(bus, device, 0, PCI_HEADER_TYPE);
            u8 max_func = 1;
            if (header_type & 0x80)
            {
                max_func = PCI_MAX_FUNC;
            }

            for (u8 func = 0; func < max_func; func++)
            {
                u16 vendor = pci_config_read_word(bus, device, func, PCI_VENDOR_ID);
                if (vendor == 0xFFFF)
                {
                    continue;
                }

                u8 cc = (u8)(pci_config_read(bus, device, func, PCI_CLASS) >> 24);
                u8 sc = (u8)((pci_config_read(bus, device, func, PCI_CLASS) >> 16) & 0xFF);

                if (cc == class_code && sc == subclass)
                {
                    pci_device_type dev;
                    pci_read_device(bus, device, func, &dev);
                    cb(&dev, ctx);
                }
            }
        }
    }
}

void pci_config_write(u8 bus, u8 device, u8 func, u8 offset, u32 value)
{
    u32 addr = pci_make_addr(bus, device, func, offset);
    asm_outl(PCI_CONFIG_ADDR, addr);
    asm_outl(PCI_CONFIG_DATA, value);
}

int pci_enable_bus_mastering(pci_device_type *dev)
{
    u32 cmd = pci_config_read(dev->bus, dev->device, dev->func, PCI_COMMAND);
    cmd |= (PCI_CMD_BUSMASTER | PCI_CMD_MEMORY | PCI_CMD_IO);
    pci_config_write(dev->bus, dev->device, dev->func, PCI_COMMAND, cmd);
    return 0;
}

int pci_bar_type(u32 bar_value)
{
    if (bar_value & 0x1)
    {
        return PCI_BAR_IO;
    }
    switch ((bar_value >> 1) & 0x3)
    {
    case 0: return PCI_BAR_MMIO;
    case 2: return PCI_BAR_MMIO_64;
    default: return PCI_BAR_MMIO;
    }
}

u64 pci_bar_address(pci_device_type *dev, int bar_index)
{
    u32 bar = dev->bar[bar_index];
    int type = pci_bar_type(bar);

    if (type == PCI_BAR_IO)
    {
        return (u64)(bar & ~0x3UL);
    }
    else if (type == PCI_BAR_MMIO_64)
    {
        if (bar_index + 1 < 6)
        {
            u32 bar_high = dev->bar[bar_index + 1];
            return ((u64)bar_high << 32) | (u64)(bar & ~0xFUL);
        }
        return (u64)(bar & ~0xFUL);
    }
    else
    {
        return (u64)(bar & ~0xFUL);
    }
}