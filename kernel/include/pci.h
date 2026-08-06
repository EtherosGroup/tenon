#ifndef KERNEL_PCI_H
#define KERNEL_PCI_H

#include "types.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_BUS 256 // 最大总线数
#define PCI_MAX_DEVICE 32 // 每条总线最多设备数
#define PCI_MAX_FUNC 8 // 每个设备最多功能单元数

// 类别编码
#define PCI_CLASS_STORAGE 0x01 // 基类：存储控制器
#define PCI_SUBCLASS_IDE 0x01 // 子类：IDE接口
#define PCI_SUBCLASS_AHCI 0x06 // 子类：AHCI接口
#define PCI_SUBCLASS_NVME 0x08 // 子类：NVMe接口

// PCI配置空间
#define PCI_VENDOR_ID 0x00 // 供应商ID
#define PCI_DEVICE_ID 0x02 // 设备ID
#define PCI_COMMAND 0x04 // 命令寄存器
#define PCI_STATUS 0x06 // 状态寄存器
#define PCI_CLASS 0x0B // 基类代码
#define PCI_SUBCLASS 0x0A // 子类代码
#define PCI_PROG_IF 0x09 // 编程接口
#define PCI_HEADER_TYPE 0x0E // 头标类型
#define PCI_BAR0 0x10 // 基地址寄存器0
#define PCI_BAR1 0x14 // 1
#define PCI_BAR2 0x18 // 2
#define PCI_BAR3 0x1C // 3
#define PCI_BAR4 0x20 // 4
#define PCI_BAR5 0x24 // 5
#define PCI_INT_LINE 0x3C // 中断线路
#define PCI_INT_PIN 0x3D // 中断引脚

// PCI 寄存器控制位
#define PCI_CMD_IO 0x0001 // Bit0 -> 启用 I/O 空间访问
#define PCI_CMD_MEMORY 0x0002 // Bit1 -> 启用 内存空间访问
#define PCI_CMD_BUSMASTER 0x0004 // Bit2 -> 启用 总线主控（DMA）能力

// 寄存器类型识别位
#define PCI_BAR_IO 0x01 // Bit 0 = 1: I/O 端口映射
#define PCI_BAR_MMIO 0x00 // Bit 0 = 0: 内存空间映射（32位）
#define PCI_BAR_MMIO_64 0x04 // Bit 2 = 1: 64位内存空间映射

typedef struct
{
    u8 bus;
    u8 device;
    u8 func;
    u16 vendor_id;
    u16 device_id;
    u8 class_code;
    u8 subclass;
    u8 prog_if;
    u8 header_type;
    u32 bar[6];
    u8 int_line;
    u8 int_pin;
} pci_device_type;

u32 pci_config_read(u8 bus, u8 device, u8 func, u8 offset);
u16 pci_config_read_word(u8 bus, u8 device, u8 func, u8 offset);
void pci_scan(void);

typedef void (*pci_device_callback_type)(pci_device_type *dev, void *ctx);
void pci_scan_class(u8 class_code, u8 subclass, pci_device_callback_type cb, void *ctx);

void pci_config_write(u8 bus, u8 device, u8 func, u8 offset, u32 value);
int pci_enable_bus_mastering(pci_device_type *dev);
int pci_bar_type(u32 bar_value);
u64 pci_bar_address(pci_device_type *dev, int bar_index);

#endif