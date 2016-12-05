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

void intel_divide_error(void)
{
    printf("INTEL EXCEPT 0: Divide by 0 error\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_debug(void)
{
    printf("INTEL EXCEPT 1: Debug exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_nmi(void)
{
    printf("INTEL EXCEPT 2: NMI interrupt\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_int3(void)
{
    printf("INTEL EXCEPT 3: Breakpoint exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_overflow(void)
{
    printf("INTEL EXCEPT 4: Overflow exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_bounds(void)
{
    printf("INTEL EXCEPT 5: Bound range exceeded\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_invalid_op(void)
{
    printf("INTEL EXCEPT 6: Invalid opcode exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_device_not_available(void)
{
    printf("INTEL EXCEPT 7: Device not available\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_doublefault_fn(void)
{
    /* Pushes Error Code */
    printf("INTEL EXCEPT 8: Double fault\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_coprocessor_seg_overrun(void)
{
    printf("INTEL EXCEPT 9: Coprocessor segment overrun\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_invalid_TSS(void)
{
    /* Pushes Error Code */
    printf("INTEL EXCEPT 10: Invalid TSS\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_seg_not_present(void)
{
    /* Pushes Error Code */
    printf("INTEL EXCEPT 11: Segment not present\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_stack_fault(void)
{
    /* Pushes Error Code */
    printf("INTEL EXCEPT 12: Stack fault\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_gpf(void)
{
    printf("INTEL EXCEPT 13: General protection exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_page_fault(void)
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
    printf("Address that was accessed (CR2): 0x%x\n", cr2);
    get_pcb()->retval = 256;
    halt(0);
}

/* 15 is Intel reserved */

void intel_fpu_coprocessor_error(void)
{
    printf("INTEL EXCEPT 16: FPU Floating Point Error\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_alignment_check(void)
{
    printf("INTEL EXCEPT 17: Alignment check exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_machine_check(void)
{
    printf("INTEL EXCEPT 18: Machine check exception\n");
    get_pcb()->retval = 256;
    halt(0);
}

void intel_simd_coprocessor_error(void)
{
    printf("INTEL EXCEPT 19: SIMD Floating Point Exception\n");
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
                SET_IDT_ENTRY(idt[i], &pit_irq);
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
    SET_IDT_ENTRY(idt[0],  &intel_divide_error);
    SET_IDT_ENTRY(idt[1],  &intel_debug);
    SET_IDT_ENTRY(idt[2],  &intel_nmi);
    SET_IDT_ENTRY(idt[3],  &intel_int3);
    SET_IDT_ENTRY(idt[4],  &intel_overflow);
    SET_IDT_ENTRY(idt[5],  &intel_bounds);
    SET_IDT_ENTRY(idt[6],  &intel_invalid_op);
    SET_IDT_ENTRY(idt[7],  &intel_device_not_available);
    SET_IDT_ENTRY(idt[8],  &intel_doublefault_fn);
    SET_IDT_ENTRY(idt[9],  &intel_coprocessor_seg_overrun);
    SET_IDT_ENTRY(idt[10], &intel_invalid_TSS);
    SET_IDT_ENTRY(idt[11], &intel_seg_not_present);
    SET_IDT_ENTRY(idt[12], &intel_stack_fault);
    SET_IDT_ENTRY(idt[13], &intel_gpf);
    SET_IDT_ENTRY(idt[14], &intel_page_fault);
    /* 15 is Intel reserved */
    SET_IDT_ENTRY(idt[16], &intel_fpu_coprocessor_error);
    SET_IDT_ENTRY(idt[17], &intel_alignment_check);
    SET_IDT_ENTRY(idt[18], &intel_machine_check);
    SET_IDT_ENTRY(idt[19], &intel_simd_coprocessor_error);
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
