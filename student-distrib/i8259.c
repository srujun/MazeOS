/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */
/* Initialize the 8259 PIC */

// static inline void io_wait(void)
// This is to have a wait between out and in 
// Returns nothing
static inline void io_wait(void)
{
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}


// void i8259_init(void)
// Save the previous status for master and slave
// Then initialise PIC with the Control words
// Returns : nothing
// Parameters : none
void
i8259_init(void)
{	
	uint8_t PIC_mask = 0xff;

	// Save the interrupt values
	master_mask = inb(MASTER_8259_PORT);
	io_wait();
	slave_mask = inb(SLAVE_8259_PORT);
	io_wait();

	// Mask the incoming interrupts prior to initialisation
	outb(PIC_mask, MASTER_8259_PORT + 1);
	io_wait();
	outb(PIC_mask, SLAVE_8259_PORT + 1);
	io_wait();

	// initialise control words for master
	outb(ICW1, MASTER_8259_PORT);
	io_wait();
	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	io_wait();
	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
	io_wait();
	outb(ICW4, MASTER_8259_PORT + 1);
	io_wait();

	// initialise control words for slave
	outb(ICW1, SLAVE_8259_PORT);
	io_wait();
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
	io_wait();
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
	io_wait();
	outb(ICW4, SLAVE_8259_PORT + 1);
	io_wait();

	// unmasking using the previously saved values
	outb(master_mask, MASTER_8259_PORT + 1);
	io_wait();
	outb(slave_mask, SLAVE_8259_PORT + 1);
	io_wait();

}

/* Enable (unmask) the specified IRQ */
// void enable_irq(uint32_t irq_num)
// Returns : none
// Parameters : none
// Enables the pins of the master or slave depending on the irq_num
void
enable_irq(uint32_t irq_num)
{
	uint8_t port;
    uint8_t value;

 	// Check if the irq_num belongs to master or slave and unmask accordingly
    if(irq_num < 8) 
    {
        port = MASTER_8259_PORT;
    } 
    else 
    {
        port = SLAVE_8259_PORT;
        irq_num -= 8;
    }
    // setting it to 0 unmasks 
    value = inb(port) & ~(1 << irq_num);
    outb(value, port); 
	
}

/* Disable (mask) the specified IRQ */
// void disable_irq(uint32_t irq_num)
// Returns : none
// Parameters : none
// Disables the pins of the master or slave depending on the irq_num
void
disable_irq(uint32_t irq_num)
{
	uint8_t port;
    uint8_t value;

 	// Check if the irq_num belongs to master or slave and unmask accordingly
    if(irq_num < 8) 
    {
        port = MASTER_8259_PORT;
    } 
    else 
    {
        port = SLAVE_8259_PORT;
        irq_num -= 8;
    }

    // setting it to 1 masks 
    value = inb(port) | (1 << irq_num);
    outb(value, port);  
}

/* Send end-of-interrupt signal for the specified IRQ */
// void send_eoi(uint32_t irq_num)
// Returns : none
// Parameters : none
// Sends End of Interrupts after checking for port number
void
send_eoi(uint32_t irq_num)
{
	uint8_t port;
    uint8_t value;

 	// Check if the irq_num belongs to master or slave
    if(irq_num < 8) 
    {
        port = MASTER_8259_PORT;
    } 
    else 
    {
        port = SLAVE_8259_PORT;
        irq_num -= 8;

    }

	value = (EOI | irq_num);
	outb(value, port);
	
	// set end of interrupt for the master as well, slave is on 2
	if(port == SLAVE_8259_PORT)
	{
		outb(EOI | 2, MASTER_8259_PORT);
	}
}

// void pic_master_irq_pit(void)
// Parameters : none
// Returns : none
// This functions handle the Programmable Interrupt Timer
void
pic_master_irq_pit(void)
{
    outb(0x20, 0x20);
    sti();
}

// void pic_master_irq_handler(void)
// Parameters : none
// Returns : none
// This handles irq for master that are not defined
void
pic_master_irq_handler(void)
{
    printf("Unimplemented master PIC IRQ!\n");
    outb(0x20, 0x20);
    sti();
}

// void pic_slave_irq_handler(void)
// Parameters : none
// Returns : none
// This handles the irqs for slave that are not defined
void
pic_slave_irq_handler(void)
{
    printf("Unimplemented slave PIC IRQ!\n");
    outb(0x20, 0x20);
    outb(0x20, 0xa0);
    sti();
}
