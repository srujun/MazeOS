/*
 * terminal.c
 * Definitions of the functions that initialize and handle
 * the terminal driver functions
 */

#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "i8259.h"


/*
 * terminal_open
 *   DESCRIPTION: Initializes the terminal and clears the screen
 *   INPUTS: filename (not used)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - success
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
 *   DESCRIPTION: Currently does nothing
 *   INPUTS: fds (not used)
 *   OUTPUTS: none
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
 *   DESCRIPTION: Gets a copy of the current keyboard input buffer
 *   INPUTS: fd - unused (0),
 *           buf - buffer to copy data into,
 *           nbytes - number of bytes to copy (unused)
 *   OUTPUTS: none
 *   RETURN VALUE: The number of bytes in the keyboard input buffer
 *   SIDE EFFECTS: keyboard input is acknowledged if control code
 *                 is present
 */
int32_t
terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    // get_kb_buffer(buf);
    // return MAX_BUFFER_SIZE;

    int32_t bytes_read = keyboard_read(fd, buf, nbytes);

    if (*((int8_t *)buf) == CTRL_L)
    {
        clear_setpos(0, 0);
        /* clear the command buffer */
        memset(buf, '\0', nbytes);
        bytes_read = 0;
    }

    return bytes_read;
}


/*
 * terminal_write
 *   DESCRIPTION: Writes to screen all characters passed in via
 *                the buf input
 *   INPUTS: fd - unused (1),
 *           buf - buffer to print to screen,
 *           nbytes - number of bytes in buffer
 *   OUTPUTS: none
 *   RETURN VALUE: zero signifying success
 *   SIDE EFFECTS: Prints the buffer to the screen
 */
int32_t
terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    disable_irq(KEYBOARD_IRQ);

    int i;
    for(i = 0; i < nbytes; i++)
    {
        putc(*(uint8_t*)buf);
        buf++;
    }

    enable_irq(KEYBOARD_IRQ);

    return nbytes;
}
