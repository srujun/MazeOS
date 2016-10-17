/*
 * rtc.c
 * This function holds the
 * initialisation and interrupt handling for the real time clock
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"


/*
 * rtc_init
 *   DESCRIPTION: Initializes the RTC, sets the rate to 15 which is the
 *                Divider Setting, sets values to ports of rtc and cmos
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables the RTC IRQ num in PIC
 */
void
rtc_init(void)
{
    char prev_saved;

    /* Select register B */
    outb(STATUS_REG_B, RTC_PORT1);

    /* Save the value from RTC_PORT2 */
    prev_saved = inb(RTC_PORT2);

    outb(STATUS_REG_B, RTC_PORT1);
    /* Enable bit 6 (Periodic interrupt) */
    outb(prev_saved | MASK_PIE , RTC_PORT2);

    /* set register STATUS_REG_A */
    outb(STATUS_REG_A, RTC_PORT1);
    prev_saved = inb(RTC_PORT2);

    outb(STATUS_REG_A, RTC_PORT1);
    /* write rate to STATUS_REG_A */
    outb((prev_saved & MASK_LOWER) | DIVIDER_SETTING, RTC_PORT2);

    enable_irq(RTC_IRQ);
}


/*
 * rtc_interrupt_handler
 *   DESCRIPTION: Handles the RTC interrupt request. Currently calls
 *                test_interrupts() to verify that the IRQ occurs. We also
 *                acknowledge IRQ 8 by setting STATUS_REG_C to ensure it can
 *                be called repeatedly.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls test_interrupts()
 */
void
rtc_interrupt_handler(void)
{
    unsigned int flags;
    cli_and_save(flags);

    /* test_interrupts(); */

    /* this is to take care of interrupts after IRQ 8 */
    outb(STATUS_REG_C, RTC_PORT1);
    inb(RTC_PORT2);

    send_eoi(RTC_IRQ);
    restore_flags(flags);
}
