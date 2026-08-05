#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include "types.h"

#define KEYBOARD_RING_SIZE 256

#define KEY_LEFT ((char)0x80)
#define KEY_RIGHT ((char)0x81)
#define KEY_DELETE ((char)0x82)
#define KEY_HOME ((char)0x83)
#define KEY_END ((char)0x84)

void keyboard_handler(void);
void keyboard_ring_init(void);
void keyboard_ring_push(char c);
char keyboard_ring_pop(void);
bool keyboard_ring_empty(void);
char keyboard_readchar(void);

#endif