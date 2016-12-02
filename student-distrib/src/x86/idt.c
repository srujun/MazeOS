/*
 * idt.c - Implementation of IDT declarations
 */

#include "x86/idt.h"
#include "syscalls/syscalls.h"
#include "drivers/rtc.h"
#include "drivers/keyboard.h"
#include "x86/i8259.h"
#include "interrupts.h"
#include "process.h"


/*
 * intel_exception_X
 *   DESCRIPTION: Functions declared using the macro INTEL_EXCEPTION. Handles
 *                functionality of exceptions when they are hit. A BSOD-like
 *                screen is displayed with the Exception message and the
 *                address of the instruction that caused it.
 *   INPUTS: none (MACRO defines no inputs)
 *   OUTPUTS: The screen is cleared and exception message is displayed
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enters an infinite loop to relieve control from user
 */
INTEL_EXCEPTION(intel_exception_0,  "EXCEPT 0: Divide by 0 error");
INTEL_EXCEPTION(intel_exception_1,  "EXCEPT 1: Debug exception");
INTEL_EXCEPTION(intel_exception_2,  "EXCEPT 2: NMI interrupt");
INTEL_EXCEPTION(intel_exception_3,  "EXCEPT 3: Breakpoint exception");
INTEL_EXCEPTION(intel_exception_4,  "EXCEPT 4: Overflow exception");
INTEL_EXCEPTION(intel_exception_5,  "EXCEPT 5: Bound range exceeded");
INTEL_EXCEPTION(intel_exception_6,  "EXCEPT 6: Invalid opcode exception");
INTEL_EXCEPTION(intel_exception_7,  "EXCEPT 7: Device not available");
INTEL_EXCEPTION(intel_exception_8,  "EXCEPT 8: Double fault");
INTEL_EXCEPTION(intel_exception_9,  "EXCEPT 9: Coprocessor segment overrun");
INTEL_EXCEPTION(intel_exception_10, "EXCEPT 10: Invalid TSS");
INTEL_EXCEPTION(intel_exception_11, "EXCEPT 11: Segment not present");
INTEL_EXCEPTION(intel_exception_12, "EXCEPT 12: Stack fault");
INTEL_EXCEPTION(intel_exception_13, "EXCEPT 13: General protection exception");
// INTEL_EXCEPTION(intel_exception_14, "EXCEPT 14: Page fault");
/* 15 is Intel reserved */
INTEL_EXCEPTION(intel_exception_16, "EXCEPT 16: FPU Floating Point Error");
INTEL_EXCEPTION(intel_exception_17, "EXCEPT 17: Alignment check exception");
INTEL_EXCEPTION(intel_exception_18, "EXCEPT 18: Machine check exception");
INTEL_EXCEPTION(intel_exception_19, "EXCEPT 19: SIMD Floating Point Exception");

void intel_page_fault()
{
    uint32_t cr2;
    asm volatile (
        "movl  %%cr2, %%eax \n\t"
        "movl  %%eax, %0"
        : "=r" (cr2)
        :
        : "%eax"
    );
    printf("INTEL EXCEPT 14: Page Fault\n");
    printf("Address that was accessed (CR2): %x\n", cr2);
    get_pcb()->retval = 256;
    halt(0);
}

/*
 * initialize_idt
 *   DESCRIPTION: Creates the descriptor entries in the Interrupt Descriptor
 *                Table. Maps the Intel Exceptions, PIC IRQs, and System Calls
 *                to their respective handler functions.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the IDT (does not load it into IDT register)
 */
void
initialize_idt() {
    int i;
    for (i = 0x0; i < NUM_VECTORS; i++)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4    = IDT_GATE_RESERVED_4;
        idt[i].reserved3    = INT_GATE_RESERVED_3;
        idt[i].reserved2    = IDT_GATE_RESERVED_2;
        idt[i].reserved1    = IDT_GATE_RESERVED_1;
        idt[i].size         = IDT_GATE_SIZE;
        idt[i].reserved0    = IDT_GATE_RESERVED_0;
        idt[i].present      = IDT_GATE_PRESENT;

        /* Intel-defined exceptions */
        if (i >= INTEL_EXCEPTIONS_START && i <= INTEL_EXCEPTIONS_END)
        {
            idt[i].dpl = IDT_GATE_SUPERVISOR;
        }
        /* 8259 PIC */
        else if (i >= PIC_IRQ_START && i <= PIC_IRQ_END)
        {
            idt[i].dpl = IDT_GATE_SUPERVISOR;

            /* keyboard */
            if (i == PIC_IRQ_START + KEYBOARD_IRQ)
                SET_IDT_ENTRY(idt[i], &keyboard_irq);
            /* rtc */
            else if (i == PIC_IRQ_START + RTC_IRQ)
                SET_IDT_ENTRY(idt[i], &rtc_irq);
            /* PIT */
            else if (i == PIC_IRQ_START + PIT_IRQ)
                SET_IDT_ENTRY(idt[i], &pic_irq_pit);
            /* unimplemented master PIC IRQs */
            else if (i <= PIC_IRQ_MASTER_END)
                SET_IDT_ENTRY(idt[i], &pic_irq_master);
            /* unimplemented slave PIC IRQs */
            else
                SET_IDT_ENTRY(idt[i], &pic_irq_slave);
        }
        /* System call */
        else if (i == SYSTEM_CALL)
        {
            idt[i].dpl = IDT_GATE_USER;
            idt[i].reserved3 = TRAP_GATE_RESERVED_3;
            SET_IDT_ENTRY(idt[i], &syscall_handler);
        }
        /* other vectors */
        else
        {
            idt[i].dpl = IDT_GATE_SUPERVISOR;
            SET_IDT_ENTRY(idt[i], &generic_interrupt_handler);
        }
    }

    /* Map all Intel exceptions in the IDT */
    SET_IDT_ENTRY(idt[0],  &intel_exception_0);
    SET_IDT_ENTRY(idt[1],  &intel_exception_1);
    SET_IDT_ENTRY(idt[2],  &intel_exception_2);
    SET_IDT_ENTRY(idt[3],  &intel_exception_3);
    SET_IDT_ENTRY(idt[4],  &intel_exception_4);
    SET_IDT_ENTRY(idt[5],  &intel_exception_5);
    SET_IDT_ENTRY(idt[6],  &intel_exception_6);
    SET_IDT_ENTRY(idt[7],  &intel_exception_7);
    SET_IDT_ENTRY(idt[8],  &intel_exception_8);
    SET_IDT_ENTRY(idt[9],  &intel_exception_9);
    SET_IDT_ENTRY(idt[10], &intel_exception_10);
    SET_IDT_ENTRY(idt[11], &intel_exception_11);
    SET_IDT_ENTRY(idt[12], &intel_exception_12);
    SET_IDT_ENTRY(idt[13], &intel_exception_13);
    // SET_IDT_ENTRY(idt[14], &intel_exception_14);
    SET_IDT_ENTRY(idt[14], &intel_page_fault);
    /* 15 is Intel reserved */
    SET_IDT_ENTRY(idt[16], &intel_exception_16);
    SET_IDT_ENTRY(idt[17], &intel_exception_17);
    SET_IDT_ENTRY(idt[18], &intel_exception_18);
    SET_IDT_ENTRY(idt[19], &intel_exception_19);
}


/*
 * generic_interrupt_handler
 *   DESCRIPTION: Handles unimplemented Interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
generic_interrupt_handler() {
    printf("Generic handler called!\n");
    return;
}
