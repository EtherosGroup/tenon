#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#ifndef null
#define null ((void*)0)
#endif

typedef enum { false = 0, true = 1 } bool;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef u64 uptr;
typedef u64 usize;

#endif