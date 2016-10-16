/*
keyboard.c
This has the definitions of the functions 
that initialise and handle the keyboard interrupts
*/

#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "keyboard.h"

// These will be used later when handling acknowledgements and resend commands
// maintain a queue of commands
uint8_t queue[MAX];  
int front;
int rear;
int size;

// This will be used later when these keys have to be handled
// bit[2] = CTRL
// bit[1] = CAPS
// bit[0] = SHIFT
uint8_t flags;      

// The keyboard requires you to have a wait in between the in and out calls
static inline void io_wait(void)
{
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

// create an array to store the ascii values of the inputs to be printed 
unsigned char scancode1[93] = {
	0, 0, '1', '2',
	'3', '4', '5', '6',
	'7', '8', '9', '0',
	'-', '=', 0 , '\t', 
	'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i',
	'o', 'p', '[', ']',
	'\n', 0, 'a', 's',
	'd', 'f', 'g', 'h',
	'j','k', 'l', ';',
	'\'', '`', 0, '\\',
	'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',',
	'.', '/', 0, '*',
	0, ' ', 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, '7',
	'8', '9', '-', '4',
	'5', '6', '+', '1',
	'2', '3', '0', '.',
	0, 0, 0, 0,
	0, 0, 0, 0,
	0
};


// void keyboard_init()
// Parameters : none
// Returns : none
// This function is used to initialize the keyboard
// The queue will be used in the future handling of interrupts
void keyboard_init()
{
    int i;
    for(i = 0; i < MAX; i++)
	   queue[i] = 0;
	front = 0;
	rear = -1;
	size = 0;
	enable_irq(KEY_IRQ);
}


// void keyboard_interrupt_handler()
// Parameters : none
// Returns : none
// This function handles the interrupts from the keyboard
// It is used to print the characters
// We mask interrupts and store the flags
// Currently we ignore Backspace, alt, ctrl, shift, delete, arrow keys, caps, numlck, scroll lck
void keyboard_interrupt_handler()
{
	int flags;
	cli_and_save(flags);
	
	uint8_t c = inb(KEYBOARD_PORT);
	io_wait();

	if(c == L_SHIFT || c == R_SHIFT)
	{
		flags = 0x01;
	}

	if (!(c & 0x80))
    {
        uint8_t output = scancode1[c];
        if(output == '\t')
        {
        	putc(' ');
        	putc(' ');
        	putc(' ');
        	putc(' ');	
        }
        else if(output == 0 || c == CURSOR_UP || c == CURSOR_RIGHT || c == CURSOR_LEFT || c == CURSOR_DOWN);
        else
    		putc(output);
    }

	send_eoi(KEY_IRQ);
	
	sti();
	restore_flags(flags);
}
