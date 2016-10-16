# interrupts.S - defines interrupt wrappers

#define ASM    1

#include "x86_desc.h"

.globl keyboard_irq
keyboard_irq:
    pushal;
    call keyboard_interrupt_handler;
    popal;
    iret;

.globl rtc_irq
rtc_irq:
    pushal;
    call rtc_interrupt_handler;
    popal;
    iret;

.globl pic_irq_pit
pic_irq_pit:
    pushal;
    call pic_master_irq_pit;
    popal;
    iret;

.globl pic_irq_master
pic_irq_master:
    pushal;
    call pic_master_irq_handler;
    popal;
    iret;

.globl pic_irq_slave
pic_irq_slave:
    pushal;
    call pic_slave_irq_handler;
    popal;
    iret;
