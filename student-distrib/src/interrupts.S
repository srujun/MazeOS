# interrupts.S - defines interrupt wrappers

#define ASM    1

#include "x86/x86_desc.h"

/*
 * irq_wrapper_X
 *   DESCRIPTION: Since each interrupt handler needs to return with the "iret"
 *                assembly instruction, we wrap our C handler functions in
 *                small assembly functions that return with "iret"
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls the respective handler function and then returns
 *                 using "iret"
 */

.globl keyboard_irq
keyboard_irq:
    pushal;
    call keyboard_interrupt_handler;
    popal;
    iret;

.globl rtc_irq
rtc_irq:
    pushal;
    call rtc_interrupt_handler;
    popal;
    iret;

.globl pic_irq_pit
pic_irq_pit:
    pushal;
    call pit_interrupt_handler;
    popal;
    iret;

.globl pic_irq_master
pic_irq_master:
    pushal;
    call pic_master_irq_handler;
    popal;
    iret;

.globl pic_irq_slave
pic_irq_slave:
    pushal;
    call pic_slave_irq_handler;
    popal;
    iret;
