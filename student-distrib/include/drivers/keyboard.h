/*
 * keyboard.h
 * Declares the initialization and interrupt handling functions for the Keyboard
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KEYBOARD_IRQ         1
#define KEYBOARD_PORT        0x60
#define KEYBOARD_STATUS_PORT 0x64
#define RELEASED_MASK        0x80

#define MAX_BUFFER_SIZE      128
#define SUPPORTED_KEYS       93
#define OFFSET_TO_UPPERCASE  32

/* Control codes */
#define CTRL_L               12
#define CTRL_C               3
#define CTRL_A               1

/* Definitions of all the keys */
#define ENTER_KEYCODE        0x1C

#define L_SHIFT_PRESSED      0x2A
#define R_SHIFT_PRESSED      0x36
#define L_SHIFT_RELEASED     0xAA
#define R_SHIFT_RELEASED     0xB6

#define CAPS_PRESSED         0x3A

#define ALT_PRESSED          0x38
#define ALT_RELEASED         0xB8

#define CTRL_PRESSED         0x1D
#define CTRL_RELEASED        0x9D
#define EXTENSION            0xE0

#define SCAN_TAB             0x0F
#define SCAN_L               0x26
#define SCAN_A               0x1E
#define SCAN_C               0x2E

#define CURSOR_UP            0x48
#define CURSOR_DOWN          0x50
#define CURSOR_LEFT          0x4B
#define CURSOR_RIGHT         0x4D

#define BACKSPACE            0x0E

/* Externally visible functions */

/* Initializes the keyboard */
void keyboard_init();

/* Handles the keyboard interrupts */
extern void keyboard_interrupt_handler();

void get_kb_buffer(void* buf);

int keyboard_read(int32_t fd, void* buf, int32_t nbytes);

int keyboard_write(int32_t fd, const void* buf, int32_t nbytes);

int keyboard_open(const uint8_t* filename);

int keyboard_close(int32_t fd);

/* local helper functions */
int32_t check_modifier_keys(uint8_t scan1, uint8_t scan2);
int32_t check_control_codes(uint8_t scan1);
void print_character(uint8_t scan1);

#endif
