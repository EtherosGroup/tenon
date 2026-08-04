#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include "types.h"

void kprint(char chars[]);

void kprintln(char chars[]);

void kprint_hex(ku64 num);

void kprint_dec(ku64 num);

#endif
