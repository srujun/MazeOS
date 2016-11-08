/* process.c - TODO */

#include "process.h"
#include "syscalls/syscalls.h"

/*
 * get_pcb
 */
pcb_t*
get_pcb()
{
    uint32_t esp;

    asm volatile("movl %%esp, %0" : "=r" (esp));
    return (pcb_t *)(esp & ESP_PCB_MASK);
}
