/* i8259.c - Functions to interact with the 8259 interrupt controller
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */
/* Initialize the 8259 PIC */


/*
 * io_wait
 *   DESCRIPTION: Writes random byte from register AL to unused port 0x80
 *                to add a wait/pause between PIC instructions
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Creates a small wait
 */
static inline void io_wait(void)
{
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}


/*
 * i8259_init
 *   DESCRIPTION: Save the previous IRQ masks for master and slave,
 *                then initialize PIC with the Interrupt Control Words (ICW)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes PIC
 */
void
i8259_init(void)
{
    uint8_t PIC_mask = 0xFF;

    /* Save the interrupt values */
    master_mask = inb(MASTER_8259_PORT);
    io_wait();
    slave_mask = inb(SLAVE_8259_PORT);
    io_wait();

    /* Mask the incoming interrupts prior to initialization */
    outb(PIC_mask, MASTER_8259_PORT + 1);
    io_wait();
    outb(PIC_mask, SLAVE_8259_PORT + 1);
    io_wait();

    /* Initialize control words for master */
    outb(ICW1, MASTER_8259_PORT);
    io_wait();
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    io_wait();
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    io_wait();
    outb(ICW4, MASTER_8259_PORT + 1);
    io_wait();

    /* Initialize control words for slave */
    outb(ICW1, SLAVE_8259_PORT);
    io_wait();
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    io_wait();
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    io_wait();
    outb(ICW4, SLAVE_8259_PORT + 1);
    io_wait();

    /* Unmasking using the previously saved values */
    outb(master_mask, MASTER_8259_PORT + 1);
    io_wait();
    outb(slave_mask, SLAVE_8259_PORT + 1);
    io_wait();

}


/*
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: uint32_t irq_num - (0-15) the IRQ number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables the pins of the master or slave depending
 *                 on the irq_num
 */
void
enable_irq(uint32_t irq_num)
{
    uint8_t port;
    uint8_t value;

     /* Check if irq_num belongs to master or slave and unmask accordingly */
    if(irq_num < PIC_MAX_PINS)
        port = MASTER_8259_PORT;
    else
    {
        port = SLAVE_8259_PORT;
        irq_num -= PIC_MAX_PINS;
    }

    /* setting it to 0 unmasks the IRQ */
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
}


/*
 * disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: uint32_t irq_num - (0-15) the IRQ number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disables the pins of the master or slave depending
 *                 on the irq_num
 */
void
disable_irq(uint32_t irq_num)
{
    uint8_t port;
    uint8_t value;

     /* Check if irq_num belongs to master or slave and unmask accordingly */
    if(irq_num < PIC_MAX_PINS)
        port = MASTER_8259_PORT;
    else
    {
        port = SLAVE_8259_PORT;
        irq_num -= PIC_MAX_PINS;
    }

    /* setting it to 1 masks the IRQ */
    value = inb(port) | (1 << irq_num);
    outb(value, port);
}


/*
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: uint32_t irq_num - (0-15) the IRQ number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends End of Interrupt after checking for port number
 */
void
send_eoi(uint32_t irq_num)
{
    uint8_t port;
    uint8_t value;

     // Check if the irq_num belongs to master or slave
    if(irq_num < PIC_MAX_PINS)
        port = MASTER_8259_PORT;
    else
    {
        port = SLAVE_8259_PORT;
        irq_num -= PIC_MAX_PINS;
    }

    value = (EOI | irq_num);
    outb(value, port);

    /* Set end of interrupt for the master as well, slave is on 2 */
    if(port == SLAVE_8259_PORT)
        outb(EOI | 2, MASTER_8259_PORT);
}


/*
 * pit_interrupt_handler
 *   DESCRIPTION: Unimplemented Programmable Interrupt Timer IRQ handler
 *                Currently just sends back acknowledgement of interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
pit_interrupt_handler(void)
{
    int flags;
    cli_and_save(flags);
    outb(UNIMPLEMENTED_ACK, MASTER_8259_PORT);
    restore_flags(flags);
}


/*
 * pic_master_irq_handler
 *   DESCRIPTION: Unimplemented handler for IRQs from the Master PIC that are
 *                not defined. Currently just sends acknowledgement of interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
pic_master_irq_handler(void)
{
    int flags;
    cli_and_save(flags);
    printf("Unimplemented master PIC IRQ!\n");
    outb(UNIMPLEMENTED_ACK, MASTER_8259_PORT);
    restore_flags(flags);
}


/*
 * pic_slave_irq_handler
 *   DESCRIPTION: Unimplemented handler for IRQs from the Slave PIC that are
 *                not defined. Currently just sends acknowledgement of interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
pic_slave_irq_handler(void)
{
    printf("Unimplemented slave PIC IRQ!\n");
    outb(UNIMPLEMENTED_ACK, MASTER_8259_PORT);
    outb(UNIMPLEMENTED_ACK, SLAVE_8259_PORT);
    sti();
}
