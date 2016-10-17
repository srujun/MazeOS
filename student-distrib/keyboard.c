/*
 * keyboard.c
 * Definitions of the functions that initialize and handle
 * the keyboard interrupts
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "keyboard.h"

/* These will be used later when handling acknowledgements and resend commands.
 * Also maintain a queue of commands received. */
uint8_t queue[MAX];
int front;
int rear;
int size;
uint8_t flags;


/* The keyboard requires you to have a wait in between the in and out calls */
static inline void io_wait(void)
{
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}


/* Create an array to store the ASCII values of the inputs to be printed */
const unsigned char scan_code_1[93] = {
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


/*
 * keyboard_init
 *   DESCRIPTION: Initializes the keyboard and local variables
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables the Keyboard IRQ num in PIC
 */
void
keyboard_init()
{
    int i;
    for(i = 0; i < MAX; i++)
        queue[i] = 0;
    front = 0;
    rear = -1;
    size = 0;
    enable_irq(KEYBOARD_IRQ);
}


/*
 * keyboard_interrupt_handler
 *   DESCRIPTION: Handles the keyboard interrupt request. It prints characters
 *                to the screen based on the data in the Keyboard Port.
 *                We mask interrupts and store the flags. Also, we ignore
 *                Backspace, alt, ctrl, shift, delete, arrow keys, caps,
 *                numlck, scroll lck.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints the character to the screen
 */
void
keyboard_interrupt_handler()
{
    int flags;
    cli_and_save(flags);

    uint8_t c = inb(KEYBOARD_PORT);
    io_wait();

    /* check if SHIFT is being pressed */
    if (c == L_SHIFT || c == R_SHIFT)
        flags = SHIFT_SET;

    /* Check if any button is being pressed */
    if (!(c & RELEASED_MASK))
    {
        uint8_t output = scan_code_1[c];
        if(output == '\t')
        {
            putc(' ');
            putc(' ');
            putc(' ');
            putc(' ');
        }
        else if(output == 0 || c == CURSOR_UP || c == CURSOR_RIGHT ||
                c == CURSOR_LEFT || c == CURSOR_DOWN);
        else
            putc(output);
    }

    send_eoi(KEYBOARD_IRQ);
    restore_flags(flags);
}
