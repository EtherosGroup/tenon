#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include "types.h"

void serial_print(const char chars[]);

void serial_println(const char chars[]);

void serial_print_hex(u64 num);

void serial_print_dec(u64 num);

#endif
