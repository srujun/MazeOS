/*
 * keyboard.h
 * Declares the initialization and interrupt handling functions for the Keyboard
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IRQ         1
#define KEYBOARD_PORT        0x60
#define KEYBOARD_STATUS_PORT 0x64
#define RELEASED_MASK        0x80

#define MAX_BUFFER_SIZE      128
#define SUPPORTED_KEYS       93
#define OFFSET_TO_UPPERCASE  32

/* Control codes */
#define CTRL_L               12

/* Definitions of all the keys */
#define ENTER_KEYCODE        0x1C

#define L_SHIFT_PRESSED      0x2A
#define R_SHIFT_PRESSED      0x36
#define L_SHIFT_RELEASED     0xAA
#define R_SHIFT_RELEASED     0xB6

#define CAPS_PRESSED         0x3A

#define CTRL_PRESSED         0x1D
#define CTRL_RELEASED        0x9D
#define EXTENSION            0xE0

#define TAB                  0x0F
#define L                    0x26

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

#endif
