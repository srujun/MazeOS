/*
 * terminal.h
 * Declares the initialization and interrupt handling functions
 * for the terminal driver
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#define NUM_COLS     80
#define NUM_ROWS     25

#define STDIN        0
#define STDOUT       1

#include "types.h"
#include "filesystem.h"

/* Externally visible functions */

/* system call for opening the terminal driver */
int32_t terminal_open(const uint8_t* filename);

/* system call for closing the terminal driver */
int32_t terminal_close(int32_t fd);

/* system call for reading from the terminal */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* system call for writing to the terminal */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
