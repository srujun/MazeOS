/*rtc.h
Used to initialise the real time clock
RTC is connected to IRQ 8
*/


#ifndef RTC_H
#define RTC_H

#define BASE_FREQUENCY 	32768	//Base frequency at which the system keeps proper time is 32.768 kHz
#define RTC_PORT1 0x70		// used to specify an index or register number
#define RTC_PORT2 0x71		// read or write from/to byte of CMOS configuration space

// Only three bytes of CMOS RAM are used with offsets at the following locations
#define STATUS_REG_A 0x8A
#define STATUS_REG_B 0x8B
#define STATUS_REG_C 0x0C

// RTC is connected to IRQ 8
#define RTC_IRQ 8

//mask values
#define PIE_MASK 0x40
#define MASK_LOWER 0xF0

// This function initialises the RTC
void rtc_init(void);

// This function handles the RTC interrupts 
extern void rtc_interrupt_handler(void);

#endif
