/*
keyboard.h 
It is the header that holds the initialisation and interrupt handling of the Keyboard
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

// These are the definitions of all the keys and commands
#define ACK 0xFA
#define RESEND 0xFE
#define MAX_TRIES 3
#define KEY_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define MAX 6

#define L_SHIFT 0x2A
#define R_SHIFT 0x36
#define CAPS 	0x3A
#define L_CTRL 	0x1D
#define R_CTRL 	0xE0

#define CURSOR_UP 0x48
#define CURSOR_DOWN 0x50
#define CURSOR_LEFT 0x4B
#define CURSOR_RIGHT 0x4D

// This function initialises the keyboard
void keyboard_init();

// This function handles the keyboard interrupts
void keyboard_interrupt_handler();

#endif
