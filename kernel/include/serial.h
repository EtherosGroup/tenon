#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include "types.h"

void serial_print(char chars[]);

void serial_println(char chars[]);

void serial_print_hex(u64 num);

void serial_print_dec(u64 num);

#endif
