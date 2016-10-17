/*
 * rtc.h
 * Used to initialise the real time clock RTC is connected to IRQ 8
 */


#ifndef RTC_H
#define RTC_H

/* Used to specify an index or register number */
#define RTC_PORT1          0x70
/* To read or write from/to byte of CMOS configuration space */
#define RTC_PORT2          0x71

/* CMOS RAM Offsets */
#define STATUS_REG_A       0x8A
#define STATUS_REG_B       0x8B
#define STATUS_REG_C       0x0C

#define RTC_IRQ            8

#define MASK_PIE           0x40
#define MASK_LOWER         0xF0

/* To turn on IRQ 8 to which the rtc is attached default rate is 1024,
   can't go beyond 15 */
#define DIVIDER_SETTING    15
/* Base frequency at which the system keeps proper time is 32.768 kHz */
#define BASE_FREQUENCY     32768

/* Externally visible functions */

void rtc_init(void);
extern void rtc_interrupt_handler(void);

#endif /* RTC_H */
