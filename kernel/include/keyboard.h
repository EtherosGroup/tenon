#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include "types.h"

#define KEYBOARD_RING_SIZE 256

void keyboard_handler(void);
void keyboard_ring_init(void);
void keyboard_ring_push(char c);
char keyboard_ring_pop(void);
bool keyboard_ring_empty(void);
char keyboard_readchar(void);

#endif