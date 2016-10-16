/*
 * idt.h - defines the data stored in the Interrupt Descriptor Table
 */

#ifndef IDT_H
#define IDT_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"

#define NUM_INTEL_EXCEPTIONS 32

#define INTEL_EXCEPTION(handler_name,desc) \
void handler_name()                        \
{                                          \
    uint32_t addr;                         \
    asm volatile("movl %%esp, %0"       \
        : "=r"(addr)                       \
    );                                     \
    clear_setpos(5, 10);                   \
    printf(desc);                          \
    printf(" at 0x%x\n", *((uint32_t *) addr + 4));             \
    while(1);                              \
}

void initialize_idt();

void generic_irq_handler();

void system_call_handler();

#endif
