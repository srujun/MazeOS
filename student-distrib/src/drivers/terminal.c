/*
 * terminal.c
 * Definitions of the functions that initialize and handle
 * the terminal driver functions
 */

#include "lib.h"
#include "paging.h"
#include "syscalls/syscalls.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "x86/i8259.h"


static volatile uint8_t curr_terminal = 0;
static terminal_t terminals[MAX_TERMINALS];

static const uint8_t attribs[MAX_TERMINALS] = {0x9F, 0x5F, 0x4F};


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
    int i, j;
    for (i = 0; i < MAX_TERMINALS; i++)
    {
        terminals[i].x_pos = 0;
        terminals[i].y_pos = 0;

        terminals[i].buffer_size = 0;
        memset(terminals[i].buffer, '\0', MAX_BUFFER_SIZE);

        terminals[i].attrib = attribs[i];

        terminals[i].ack = 0;
        terminals[i].read_ack = 0;

        for (j = 0; j < MAX_PROCESSES; j++)
            terminals[i].child_procs[j] = NULL;

        terminals[i].num_procs = 0;

        terminals[i].virt_vidmem_backup =
                    (uint8_t *)(VID_BKUP_MEM_START_VIRT + (i * _4KB));
        terminals[i].phys_vidmem_backup =
                    (VID_BKUP_MEM_START_PHYS + (i * _4KB));
        /* create mapping for backup video memory pages */
        map_backup_vidmem((uint32_t)(terminals[i].virt_vidmem_backup),
                          terminals[i].phys_vidmem_backup);

        /* write the color data to the backup memory */
        uint32_t drawval = (attribs[i] << 8) | ' ';
        memset_word(terminals[i].virt_vidmem_backup, drawval, _4KB / 2);
    }

    curr_terminal = 0;
}


/*
 * active_term TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
terminal_t *
active_term()
{
    return &terminals[curr_terminal];
}


/*
 * active_term TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
terminal_t *
get_term(uint32_t term_num)
{
    if (term_num >= MAX_TERMINALS)
    {
        /* should never come in here... */
        printf("Terminal does not exist\n");
        cli();
        while(1);
    }
    return &terminals[term_num];
}


/*
 * switch_active_terminal TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
switch_active_terminal(uint32_t term_num)
{
    cli();

    if (term_num >= MAX_TERMINALS)
    {
        /* should never come in here... */
        printf("Terminal does not exist\n");
        cli();
        while(1);
    }

    if (active_term()->num_procs != 0 && curr_terminal == term_num)
        return;

    uint32_t term_procs = active_term()->num_procs;

    /* backup video memory */
    memcpy(active_term()->virt_vidmem_backup, (void *)VIDEO_MEM_START, _4KB);
    /* save current screen position */
    active_term()->x_pos = get_screen_x();
    active_term()->y_pos = get_screen_y();

    /* if vidmap was created for current terminal, map that to backup */
    if (active_term()->child_procs[term_procs-1]->vidmem_virt_addr != 0)
    {
        pte_t pte;
        memset(&(pte), 0, sizeof(pte_t));

        /* create the video memory page with user access */
        pte.present = 1;
        pte.read_write = 1;
        pte.user_supervisor = 1;
        pte.base_addr = active_term()->phys_vidmem_backup >> SHIFT_4KB;
        map_user_video_mem(
            active_term()->child_procs[term_procs-1]->vidmem_virt_addr, pte);
    }

    /* change current terminal number */
    curr_terminal = term_num;

    /* update terminal processes */
    term_procs = active_term()->num_procs;

    /* load the new terminal's backed up screen */
    clear_setpos(active_term()->x_pos, active_term()->y_pos);
    memcpy((void *)VIDEO_MEM_START, active_term()->virt_vidmem_backup, _4KB);
    
    /* execute new shell if no process exists in this terminal */
    if (active_term()->num_procs == 0)
        execute((uint8_t *)"shell");
    else
    {
        /* if vidmap was created for next terminal process, restore mapping */
        if (active_term()->child_procs[term_procs-1]->vidmem_virt_addr != 0)
        {
            pte_t pte;
            memset(&(pte), 0, sizeof(pte_t));

            /* create the video memory page with user access */
            pte.present = 1;
            pte.read_write = 1;
            pte.user_supervisor = 1;
            pte.base_addr = VIDEO_MEM_INDEX;
            map_user_video_mem(
                active_term()->child_procs[term_procs-1]->vidmem_virt_addr, pte);
        }
    }

    sti();
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
