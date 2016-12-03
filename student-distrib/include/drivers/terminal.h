/*
 * terminal.h
 * Declares the initialization and interrupt handling functions
 * for the terminal driver
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "lib.h"
#include "filesystem.h"
#include "process.h"
#include "drivers/keyboard.h"

#define MAX_TERMINALS  3

#define NUM_COLS       80
#define NUM_ROWS       25

#define STDIN          0
#define STDOUT         1

typedef struct terminal {
    uint32_t x_pos;
    uint32_t y_pos;
    uint8_t * virt_vidmem_backup;

    uint32_t buffer_size;
    uint8_t buffer[MAX_BUFFER_SIZE];

    uint8_t attrib;

    volatile uint8_t ack;
    volatile uint8_t read_ack;

    pcb_t * child_procs[MAX_PROCESSES];
    uint32_t num_procs;
} terminal_t;

/* Externally visible functions */
terminal_t * active_term();
terminal_t * get_term(uint32_t term_num);
void switch_active_terminal(uint32_t term_num);
void terminal_init();

/* System calls */
/* system call for opening the terminal driver */
int32_t terminal_open(const uint8_t* filename);
/* system call for closing the terminal driver */
int32_t terminal_close(int32_t fd);
/* system call for reading from the terminal */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
/* system call for writing to the terminal */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
