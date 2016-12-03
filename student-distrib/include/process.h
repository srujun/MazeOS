/* process.h - Declaration of process functions */

#ifndef PROCESS_H
#define PROCESS_H

#include "lib.h"
#include "paging.h"
#include "filesystem.h"

#define ESP_PCB_MASK           0xFFFFE000
#define MAX_PROCESSES          6
#define ARGS_LENGTH            128
#define RETURN_EXCEPTION       256

/* PIT Constants */
#define PIT_CHANNEL0_REG       0x40
#define PIT_CMD_REG            0x43

#define PIT_BASE_FREQ          1193182  // Hz
/* Mode 3 (square wave), select channel 0 (OSDev: PIT) */
#define PIT_CMD_VAL            0x36
/* 25 ms = 40 Hz */
#define PIT_25MS               (PIT_BASE_FREQ / 40)

typedef struct pcb pcb_t;
struct pcb {
    uint32_t esp0;
    uint16_t pid;

    int32_t retval;

    pde_4M_t pde;
    uint32_t pde_virt_addr;
    uint32_t vidmem_virt_addr;

    uint32_t esp;
    uint32_t ebp;
    uint32_t k_esp;
    uint32_t k_ebp;

    file_desc_t fds[MAX_OPEN_FILES];
    uint8_t args[ARGS_LENGTH];
    uint32_t args_length;

    pcb_t * parent;
};

/* External functions */
pcb_t* get_pcb();

int32_t get_available_pid();

int32_t free_pid(uint32_t pid);

/* Scheduling functions */
void pit_init(void);

void context_switch(void);

/* Handles the programmable interrupt timer (PIT) interrupts */
extern void pit_interrupt_handler(void);

#endif
