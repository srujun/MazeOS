/*
 * idt.h - defines the data stored in the Interrupt Descriptor Table
 */

#ifndef IDT_H
#define IDT_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"

#define NUM_VECTORS              256

#define INTEL_EXCEPTIONS_START   0x00
#define INTEL_EXCEPTIONS_END     0x1F

#define PIC_IRQ_START            0x20
#define PIC_IRQ_END              0x2F
#define PIC_IRQ_MASTER_END       0x27

#define SYSTEM_CALL              0x80

#define IDT_GATE_RESERVED_0      0
#define IDT_GATE_RESERVED_1      1
#define IDT_GATE_RESERVED_2      1
#define IDT_GATE_RESERVED_4      0
#define IDT_GATE_SIZE            1
#define IDT_GATE_SUPERVISOR      0
#define IDT_GATE_USER            3
#define IDT_GATE_PRESENT         1
#define IDT_GATE_NOT_PRESENT     0

#define INT_GATE_RESERVED_3      0
#define TRAP_GATE_RESERVED_3     1

#define INTEL_EXCEPTION(handler_name,desc)            \
void handler_name()                                   \
{                                                     \
    clear_setpos(5, 10);                              \
    printf(desc);                                     \
    while(1);                                         \
}

/* External functions */

void initialize_idt();

void generic_interrupt_handler();

void system_call_handler();

#endif
