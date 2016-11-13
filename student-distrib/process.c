/* process.c - TODO */

#include "process.h"
#include "syscalls/syscalls.h"

static uint8_t available_pids[MAX_PROCESSES] = {0};


/*
 * TODO: get_pcb
 */
pcb_t*
get_pcb()
{
    uint32_t esp;

    asm volatile("movl %%esp, %0" : "=r" (esp));
    return (pcb_t *)(esp & ESP_PCB_MASK);
}


/*
 * TODO: get_available_pid
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
 * TODO: free_pid
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
