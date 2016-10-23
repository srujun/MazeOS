/*
 * keyboard.c
 * Definitions of the functions that initialize and handle
 * the keyboard interrupts
 */

#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "i8259.h"


/*
 * terminal_open
 *   DESCRIPTION: Initializes the keyboard and local variables
 *   INPUTS: filename (not used)
 *   OUTPUTS: int32_t
 *   RETURN VALUE: zero signifying success
 *   SIDE EFFECTS: clears screen and sets cursor to the start of screen
 */
int32_t 
terminal_open(const uint8_t* filename)
{
    clear_setpos(0, 0);
    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: Initializes the keyboard and local variables
 *   INPUTS: fds (not used)
 *   OUTPUTS: int32_t
 *   RETURN VALUE: zero signifying success
 *   SIDE EFFECTS: none
 */
int32_t 
terminal_close(int32_t fd)
{
    return 0;
}

/*
 * terminal_read
 *   DESCRIPTION: Initializes the keyboard and local variables
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: int32_t
 *   RETURN VALUE: zero signifying success
 *   SIDE EFFECTS: none
 */
int32_t 
terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    return 0;
}


/*
 * terminal_write
 *   DESCRIPTION: Writes to screen all characters passed in via
 *                the buf input, handles cases such as backspace
 *                and enter, moves to the next line when needed,
 *                and implements scrolling downwards as needed
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: int32_t
 *   RETURN VALUE: zero signifying success
 *   SIDE EFFECTS: Prints the character to the screen
 */
int32_t 
terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    disable_irq(KEYBOARD_IRQ);

    int i;
    int8_t char_arr[256];
    char_arr[nbytes] = '\0';
    memcpy(char_arr, buf, nbytes);
    puts(char_arr);

    enable_irq(KEYBOARD_IRQ);

    return 0;
}
