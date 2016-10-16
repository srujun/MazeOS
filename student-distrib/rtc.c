/*
rtc.c
This  function holds the
initialisation and interrupt handling for
the real time clock 
*/

#include "rtc.h"
#include "i8259.h"
#include "lib.h"

// void rtc_init(void)
// Parameters : none
// Returns : none
// Set the rate to 15 which is the Divider Setting
// Initialises the rtc 
// sets values to ports of rtc and cmos
void rtc_init(void)
{
	// To turn on IRQ 8 to which the rtc is attached
	// default rate is 1024
	// can't go beyond 15
	int rate = 15;
	char prev_saved;
	outb(STATUS_REG_B, RTC_PORT1); // select register B
	prev_saved = inb(RTC_PORT2); // save the value from port 0x71
	outb(STATUS_REG_B, RTC_PORT1);
	outb(prev_saved | PIE_MASK , RTC_PORT2); //enable bit 6 (Periodic interrupt)

	outb(STATUS_REG_A, RTC_PORT1); // set register A
	prev_saved = inb(RTC_PORT2);
	outb(STATUS_REG_A, RTC_PORT1);
	outb((prev_saved & MASK_LOWER) | rate, RTC_PORT2); // write rate to A

	enable_irq(RTC_IRQ); // enabling the interrupts
}

// void rtc_interrupt_handler(void)
// Parameters : none
// Returns : none
// This handles the rtc interrupts
// Handle register c to get interrupts after irq 8 
// as if after irq 8 , register c not read will imply
// interrupts don't occur
void rtc_interrupt_handler(void)
{
	unsigned int flags;
	cli_and_save(flags);
	//test_interrupts();
	restore_flags(flags);
	// this is to take care of interrupts after irq 8
    outb(STATUS_REG_C, RTC_PORT1);
    inb(RTC_PORT2);
    // send EOI
    send_eoi(RTC_IRQ);
}
