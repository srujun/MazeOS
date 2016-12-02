/*
 * terminal.c
 * Definitions of the functions that initialize and handle
 * the terminal driver functions
 */

#include "lib.h"
#include "paging.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "x86/i8259.h"


static volatile uint8_t curr_terminal = 0;
static terminal_t terminals[MAX_TERMINALS];


/*
 * terminal_init TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
terminal_init()
{
    int i;
    for (i = 0; i < MAX_TERMINALS; i++)
    {
        terminals[i].x_pos = terminals[i].y_pos = 0;
        terminals[i].virt_vidmem_backup = (uint8_t *)(_64MB + (i * _4KB));

        terminals[i].buffer_size = 0;
        memset(terminals[i].buffer, '\0', MAX_BUFFER_SIZE);

        terminals[i].ack = 0;
        terminals[i].read_ack = 0;

        /* create mapping for backup video memory pages */
        map_backup_vidmem(VID_BKUP_MEM_START_VIRT + (i * _4KB),
                          VID_BKUP_MEM_START_PHYS + (i * _4KB));
    }

    curr_terminal = 0;
}


/*
 * get_curr_terminal TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
terminal_t *
get_curr_terminal()
{
    return &terminals[curr_terminal];
}


/*
 * get_curr_terminal TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
switch_active_terminal(uint8_t from, uint8_t to)
{
    /* backup video memory */
    // change paging -> 
    curr_terminal = to;
}


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
