#ifndef KERNEL_KHEAP_H
#define KERNEL_KHEAP_H

#include "types.h"

#define KHEAP_MIN_ALLOC 16

void  kheap_init(void);

void *kmalloc(ku64 size);

void  kfree(void *ptr);

void *krealloc(void *ptr, ku64 new_size);

#endif