/*
 * interrupts.h - Defines Assembly wrappers around interrupt handler functions
 */


#ifndef INTERRUPTS_H
#define INTERRUPTS_H

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

extern void keyboard_irq(void);

extern void rtc_irq(void);

extern void pic_irq_pit(void);

extern void pic_irq_master(void);

extern void pic_irq_slave(void);

#endif
