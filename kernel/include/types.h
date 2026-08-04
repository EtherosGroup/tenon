#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#ifndef null
#define null ((void*)0)
#endif

typedef enum { false = 0, true = 1 } kbool;

typedef unsigned char ku8;
typedef unsigned short ku16;
typedef unsigned int ku32;
typedef unsigned long long ku64;

typedef signed char ks8;
typedef signed short ks16;
typedef signed int ks32;
typedef signed long long ks64;

typedef ku64 kuptr;
typedef ku64 ksize_t;

#endif