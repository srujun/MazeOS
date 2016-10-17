/*
 * keyboard.h
 * Declares the initialization and interrupt handling functions for the Keyboard
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IRQ         1
#define KEYBOARD_PORT        0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Definitions of all the commands */
#define ACK                  0xFA
#define RESEND               0xFE
#define MAX_TRIES            3
#define MAX                  6
#define RELEASED_MASK        0x80

/* Definitions of all the keys */
#define L_SHIFT              0x2A
#define R_SHIFT              0x36
#define CAPS                 0x3A
#define L_CTRL               0x1D
#define R_CTRL               0xE0
#define SHIFT_SET            1

#define CURSOR_UP 0x48
#define CURSOR_DOWN 0x50
#define CURSOR_LEFT 0x4B
#define CURSOR_RIGHT 0x4D

/* Externally visible functions */

/* Initializes the keyboard */
void keyboard_init();

/* Handles the keyboard interrupts */
extern void keyboard_interrupt_handler();

#endif
