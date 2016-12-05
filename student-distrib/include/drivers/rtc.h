/*
 * rtc.h
 * Used to initialise the real time clock RTC is connected to IRQ 8
 */


#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "filesystem.h"

/* Used to specify an index or register number */
#define RTC_PORT1          0x70
/* To read or write from/to byte of CMOS configuration space */
#define RTC_PORT2          0x71

/* CMOS RAM Offsets */
#define STATUS_REG_A       0x8A
#define STATUS_REG_B       0x8B
#define STATUS_REG_C       0x0C

#define RTC_IRQ               8

#define MASK_PIE           0x40
#define MASK_LOWER         0xF0

/* Values for the divider setting to set the frequency on the RTC */
#define DIVIDER_SETTING_CEIL 16
#define DIVIDER_SETTING 	 15

#define RTC_MAX_FREQ        1024
#define RTC_MIN_FREQ        2


/* Externally visible functions */

void rtc_init(void);

extern void rtc_interrupt_handler(void);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_open(const uint8_t* filename);

int32_t rtc_close(int32_t fd);

#endif /* RTC_H */
