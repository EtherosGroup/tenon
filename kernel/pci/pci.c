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