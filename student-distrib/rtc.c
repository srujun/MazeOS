/*
 * rtc.c
 * This function holds the
 * initialisation and interrupt handling for the real time clock
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"

volatile int rtc_interrupt_occurred = 0;

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
    disable_irq(RTC_IRQ);

    /* test_interrupts(); */
    rtc_interrupt_occurred = 1;

    /* this is to ensure Register C is read after IRQ 8 */
    outb(STATUS_REG_C, RTC_PORT1);
    inb(RTC_PORT2);

    send_eoi(RTC_IRQ);
    enable_irq(RTC_IRQ);
}

/*
 * rtc_read
 *   DESCRIPTION: Waits until an interrupt has occured.
 *   INPUTS: fd     - ignored
 *           buf    - ignored
 *           nbytes - ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t
rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
    while(!rtc_interrupt_occurred);

    rtc_interrupt_occurred = 0;
    return 0;
}

/*
 * rtc_write
 *   DESCRIPTION: Updates the frequency of the RTC interrupts
 *                according to the given input.
 *   INPUTS: fd     - ignored
 *           buf    - frequency to set
 *           nbytes - number of bytes being passed
 *   OUTPUTS: none
 *   RETURN VALUE: 0  - if succesful
 *                 -1 - otherwise
 *   SIDE EFFECTS: Changes frequency of the RTC interrupts
 */
int32_t
rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
    uint8_t set_divider;
    uint32_t freq;
    uint8_t count;
    char prev_saved;

    if(nbytes != 4)
        return -1;

    memcpy(&freq, buf, sizeof(uint32_t));
    
    count = 0;
    
    /* set register STATUS_REG_A */
    outb(STATUS_REG_A, RTC_PORT1);
    prev_saved = inb(RTC_PORT2);

    if(freq <= 0 || freq >= 1024)
        return -1;

    while(freq != 1)
    {
        if(freq % 2 != 0)
            return -1;
        
        freq = freq/2;
        count++;
    }
    
    set_divider = DIVIDER_SETTING_CEIL - count;

    outb(STATUS_REG_A, RTC_PORT1);
    /* write rate to STATUS_REG_A */
    outb((prev_saved & MASK_LOWER) | set_divider, RTC_PORT2);

    return 0;
}

/*
 * rtc_open
 *   DESCRIPTION: Currently, does nothing
 *   INPUTS: filename - ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t
rtc_open(const uint8_t* filename)
{
    return 0;
}

/*
 * rtc_close
 *   DESCRIPTION: Currently, does nothing
 *   INPUTS: fd - ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t
rtc_close(int32_t fd)
{
    return 0;
}
