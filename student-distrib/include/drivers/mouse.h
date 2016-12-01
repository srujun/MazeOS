/*
 * mouse.h
 * Declares the initialization and interrupt handling functions for the Mouse
 */

#ifndef MOUSE_H
#define MOUSE_H

#include "../types.h"

#define MOUSE_IRQ            12
#define MOUSE_PORT           0x60
#define MOUSE_STATUS_PORT    0x64

#define ENABLE_PACKETS       0xF4
#define WRITE_CMD            0xD4

#define SIGNED_OR            0xFFFFFF00
/* Externally visible functions */

/* Initializes the Mouse*/
void mouse_init();

/* Handles the mouse interrupts */
extern void mouse_interrupt_handler();

int mouse_read(int32_t fd, void* buf, int32_t nbytes);

int mouse_write(int32_t fd, const void* buf, int32_t nbytes);

int mouse_open(const uint8_t* filename);

int mouse_close(int32_t fd);

/* local helper functions */

#endif
