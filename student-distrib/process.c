/* process.c - Implementations of process functions */

#include "process.h"
#include "syscalls/syscalls.h"

static uint8_t available_pids[MAX_PROCESSES] = {0};


/*
 * get_pcb
 *   DESCRIPTION: Get the address of the PCB for the current process in kmemory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Pointer to the pcb as a pcb_t
 *   SIDE EFFECTS: none
 */
pcb_t*
get_pcb()
{
    uint32_t esp;

    asm volatile("movl %%esp, %0" : "=r" (esp));
    return (pcb_t *)(esp & ESP_PCB_MASK);
}


/*
 * get_available_pid
 *   DESCRIPTION: Finds and sets an unused Process ID
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: The available PID number
 *                 -1 if unavailable
 *   SIDE EFFECTS: Sets that PID number to used
 */
int32_t
get_available_pid()
{
    uint32_t i;
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        if (available_pids[i] == 0)
        {
            available_pids[i] = 1;
            return i + 1;
        }
    }

    return -1;
}


/*
 * free_pid
 *   DESCRIPTION: Marks a given PID to unused
 *   INPUTS: The PID to mark
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - if successful
 *                 -1 - if PID is already marked free
 *   SIDE EFFECTS: none
 */
int32_t
free_pid(uint32_t pid)
{
    if (pid == 0 || pid > MAX_PROCESSES)
        return -1;

    if (available_pids[pid - 1] == 0)
        return -1;

    available_pids[pid - 1] = 0;
    return 0;
}
