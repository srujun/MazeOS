/* process.h - Declaration of process functions */

#ifndef PROCESS_H
#define PROCESS_H

#include "lib.h"
#include "paging.h"
#include "filesystem.h"

#define ESP_PCB_MASK           0xFFFFE000
#define MAX_PROCESSES          2
#define ARGS_LENGTH            128
#define RETURN_EXCEPTION       256

typedef struct pcb pcb_t;
struct pcb {
    uint32_t esp0;
    uint16_t pid;

    int32_t retval;

    pde_4M_t pde;
    uint32_t pde_virt_addr;

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

#endif
