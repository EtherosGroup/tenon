#ifndef KERNEL_ATA_H
#define KERNEL_ATA_H

#include "types.h"
#include "block.h"

#define ATA_SECTOR_SIZE 512 // 扇区标准字节数

// ATA 主通道 I/O 端口地址 
#define ATA_PRIMARY_DATA 0x1F0 // 数据寄存器
#define ATA_PRIMARY_ERROR 0x1F1 // 错误寄存器
#define ATA_PRIMARY_FEATURES 0x1F1 // 特征寄存器
#define ATA_PRIMARY_SECCOUNT 0x1F2 // 扇区计数寄存器
#define ATA_PRIMARY_LBA_LO 0x1F3 // LBA低8位
#define ATA_PRIMARY_LBA_MID 0x1F4 // LBA中8位
#define ATA_PRIMARY_LBA_HI 0x1F5 // LBA高8位
#define ATA_PRIMARY_DRIVE 0x1F6 // 驱动/磁头寄存器
#define ATA_PRIMARY_STATUS 0x1F7 // 状态寄存器
#define ATA_PRIMARY_COMMAND 0x1F7 // 命令寄存器
#define ATA_PRIMARY_CTRL 0x3F6 // 控制/备用状态寄存器

// ATA 从通道 I/O 端口地址
#define ATA_SECONDARY_DATA 0x170 // 数据寄存器
#define ATA_SECONDARY_ERROR 0x171 // 错误寄存器
#define ATA_SECONDARY_FEATURES 0x171 // 特征寄存器
#define ATA_SECONDARY_SECCOUNT 0x172 // 扇区计数寄存器
#define ATA_SECONDARY_LBA_LO 0x173 // LBA低8位
#define ATA_SECONDARY_LBA_MID 0x174 // LBA中8位
#define ATA_SECONDARY_LBA_HI 0x175 // LBA高8位
#define ATA_SECONDARY_DRIVE 0x176 // 驱动/磁头寄存器
#define ATA_SECONDARY_STATUS 0x177 // 状态寄存器
#define ATA_SECONDARY_COMMAND 0x177 // 命令寄存器
#define ATA_SECONDARY_CTRL 0x376 // 控制/备用状态寄存器

// ATA 状态寄存器关键标志位
#define ATA_SR_BSY 0x80 // Bit7 - 忙
#define ATA_SR_DRDY 0x40 // Bit6 - 设备就绪
#define ATA_SR_DF 0x20 // Bit5 - 设备故障
#define ATA_SR_DRQ 0x08 // Bit3 - 数据请求
#define ATA_SR_ERR 0x01 // Bit0 - 错误，上一条命令执行失败

// ATA 标准核心命令码
#define ATA_CMD_IDENTIFY 0xEC // 识别设备
#define ATA_CMD_READ_PIO_EXT 0x24 // 读取扇区扩展
#define ATA_CMD_WRITE_PIO_EXT 0x34 // 写入扇区扩展

// ATA DRIVE 寄存器
#define ATA_DRIVE_MASTER 0xA0 // 选择主设备
#define ATA_DRIVE_SLAVE 0xB0 // 选择从设备

block_device_type *ata_init(u16 base, u16 ctrl, u8 drive_select, const char *name);

#endif