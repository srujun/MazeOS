/*
 * idt.c - Implementation of IDT declarations
 */

#include "idt.h"
#include "rtc.h"
#include "i8259.h"
#include "interrupts.h"

INTEL_EXCEPTION(intel_exception_0, "EXCEPT: Divide by 0 error");
INTEL_EXCEPTION(intel_exception_1, "EXCEPT: Debug exception");
INTEL_EXCEPTION(intel_exception_2, "EXCEPT: NMI interrupt");
INTEL_EXCEPTION(intel_exception_3, "EXCEPT: Breakpoint exception");
INTEL_EXCEPTION(intel_exception_4, "EXCEPT: Overflow exception");
INTEL_EXCEPTION(intel_exception_5, "EXCEPT: Bound range exceeded");
INTEL_EXCEPTION(intel_exception_6, "EXCEPT: Invalid opcode exception");
INTEL_EXCEPTION(intel_exception_7, "EXCEPT: Device not available");
INTEL_EXCEPTION(intel_exception_8, "EXCEPT: Double fault");
INTEL_EXCEPTION(intel_exception_9, "EXCEPT: Coprocessor segment overrun");
INTEL_EXCEPTION(intel_exception_10, "EXCEPT: Invalid TSS");
INTEL_EXCEPTION(intel_exception_11, "EXCEPT: Segment not present");
INTEL_EXCEPTION(intel_exception_12, "EXCEPT: Stack fault");
INTEL_EXCEPTION(intel_exception_13, "EXCEPT: General protection exception");
INTEL_EXCEPTION(intel_exception_14, "EXCEPT: Page fault");
/* 15 is Intel reserved */
INTEL_EXCEPTION(intel_exception_16, "EXCEPT: FPU Floating Point Error");
INTEL_EXCEPTION(intel_exception_17, "EXCEPT: Alignment check exception");
INTEL_EXCEPTION(intel_exception_18, "EXCEPT: Machine check exception");
INTEL_EXCEPTION(intel_exception_19, "EXCEPT: SIMD Floating Point Exception");

void 
initialize_idt() {
    int i;
    for (i = 0x0; i < 0xFF; i++)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4    = 0x00;
        idt[i].reserved3    = 0x0;
        idt[i].reserved2    = 0x1;
        idt[i].reserved1    = 0x1;
        idt[i].size         = 0x1;
        idt[i].reserved0    = 0x0;
        idt[i].present      = 0x1;

        /* Intel-defined exceptions */
        if (i >= 0x0 && i <= 0x1F)
        {
            idt[i].dpl = 0x0;
            // SET_IDT_ENTRY(idt[i], &intel_exception_handler);
        }
        /* 8259 PIC */
        else if (i >= 0x20 && i <= 0x2F)
        {
            idt[i].dpl = 0x0;
            if (i == 0x21) // keyboard
                SET_IDT_ENTRY(idt[i], &keyboard_irq);
            else if (i == 0x28) // rtc
                SET_IDT_ENTRY(idt[i], &rtc_irq);
            else if (i == 0x20)
                SET_IDT_ENTRY(idt[i], &pic_irq_pit);
            else if (i <= 0x27)
                SET_IDT_ENTRY(idt[i], &pic_irq_master);
            else
                SET_IDT_ENTRY(idt[i], &pic_irq_slave);
        }
        /* System call */
        else if (i == 0x80)
        {
            idt[i].dpl = 0x3;
            SET_IDT_ENTRY(idt[i], &system_call_handler);
        }
        /* other vectors */
        else
        {
            idt[i].dpl = 0x0;
            SET_IDT_ENTRY(idt[i], &generic_irq_handler);
        }
    }

    SET_IDT_ENTRY(idt[0], &intel_exception_0);
    SET_IDT_ENTRY(idt[1], &intel_exception_1);
    SET_IDT_ENTRY(idt[2], &intel_exception_2);
    SET_IDT_ENTRY(idt[3], &intel_exception_3);
    SET_IDT_ENTRY(idt[4], &intel_exception_4);
    SET_IDT_ENTRY(idt[5], &intel_exception_5);
    SET_IDT_ENTRY(idt[6], &intel_exception_6);
    SET_IDT_ENTRY(idt[7], &intel_exception_7);
    SET_IDT_ENTRY(idt[8], &intel_exception_8);
    SET_IDT_ENTRY(idt[9], &intel_exception_9);
    SET_IDT_ENTRY(idt[10], &intel_exception_10);
    SET_IDT_ENTRY(idt[11], &intel_exception_11);
    SET_IDT_ENTRY(idt[12], &intel_exception_12);
    SET_IDT_ENTRY(idt[13], &intel_exception_13);
    SET_IDT_ENTRY(idt[14], &intel_exception_14);
    /* 15 is Intel reserved */
    SET_IDT_ENTRY(idt[16], &intel_exception_16);
    SET_IDT_ENTRY(idt[17], &intel_exception_17);
    SET_IDT_ENTRY(idt[18], &intel_exception_18);
    SET_IDT_ENTRY(idt[19], &intel_exception_19);
}

void
generic_irq_handler() {
    printf("Generic handler called!\n");
    return;
}

// void
// intel_exception_handler()
// {
//     printf("Intel Exception!\n");
//     return;
// }

void
system_call_handler()
{
    printf("System Call!\n");
    return;
}
