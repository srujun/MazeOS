
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

extern void keyboard_irq(void);

extern void rtc_irq(void);

extern void pic_irq_pit(void);

extern void pic_irq_master(void);

extern void pic_irq_slave(void);

#endif
