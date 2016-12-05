/*
 * keyboard.c
 * Definitions of the functions that initialize and handle
 * the keyboard interrupts
 */

#include "drivers/keyboard.h"
#include "drivers/rtc.h"
#include "drivers/terminal.h"
#include "x86/i8259.h"
#include "lib.h"

/* flags keeps tracks of all the special keys */
static uint8_t l_shift;
static uint8_t r_shift;
static uint8_t caps;
static uint8_t l_ctrl;
static uint8_t r_ctrl;
static uint8_t l_alt;
static uint8_t r_alt;

/* Create an array to store the ASCII values of the inputs to be printed */
const static unsigned char scan_code_1[2][SUPPORTED_KEYS] = {
    {
        0, 0, '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=', 8 , 9,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
        13, 17, 'a', 's',
        'd', 'f', 'g', 'h',
        'j','k', 'l', ';',
        '\'', '`', 16, '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/', 16, 0,
        0, ' ', 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0
    },
    {
        0, 0, '!', '@',
        '#', '$', '%', '^',
        '&', '*', '(', ')',
        '_', '+', 8 , 9,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '{', '}',
        13, 17, 'a', 's',
        'd', 'f', 'g', 'h',
        'j','k', 'l', ':',
        '\"', '~', 16, '|',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', '<',
        '>', '?', 16, 0,
        0, ' ', 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0
    }
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
    /* clear the buffer */
    memset(active_term()->buffer, '\0', MAX_BUFFER_SIZE);

    active_term()->buffer_size = 0;
    active_term()->ack = 0;
    active_term()->read_ack = 0;
    l_shift = 0;
    r_shift = 0;
    caps = 0;
    l_ctrl = 0;
    r_ctrl = 0;
    l_alt = 0;
    r_alt = 0;

    enable_irq(KEYBOARD_IRQ);
}


/*
 * keyboard_interrupt_handler
 *   DESCRIPTION: Handles the keyboard interrupt request. It prints characters
 *                to the screen based on the data in the Keyboard Port.
 *                We mask interrupts and store the flags.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints the character to the screen, and fills the buffer
 */
void
keyboard_interrupt_handler()
{
    disable_irq(KEYBOARD_IRQ);

    uint8_t c = inb(KEYBOARD_PORT);
    uint8_t c2 = inb(KEYBOARD_PORT);

    /* check modifier keys like SHIFT, CTRL, ALT, and CAPS LOCK */
    if (check_modifier_keys(c, c2))
    {
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }

    if (check_function_keys(c))
    {
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }

    /* check if user has pressed any control code combinations */
    if (active_term()->read_ack)
    {
        if (check_control_codes(c))
        {
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }
    }

    /* Check if any button is being pressed */
    if (!(c & RELEASED_MASK))
    {
        /* handle backspace input */
        if(c == BACKSPACE)
        {
            if (active_term()->buffer_size == 0)
            {
                /* don't do backspace if buffer is empty */
                send_eoi(KEYBOARD_IRQ);
                enable_irq(KEYBOARD_IRQ);
                return;
            }
            if (active_term()->read_ack)
            {
                active_term()->buffer_size--;
                active_term()->buffer[active_term()->buffer_size] = '\0';
                do_backspace();
            }
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* handle enter key */
        if(c == ENTER_KEYCODE)
        {
            if (active_term()->read_ack)
            {
                active_term()->buffer[active_term()->buffer_size] = '\n';
                active_term()->buffer_size++;
            }
            active_term()->ack = 1;
            putc('\n');
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* otherwise print the character */
        print_character(c);

        /* check for buffer filled */
        if(active_term()->buffer_size == MAX_BUFFER_SIZE - 2)
        {
            active_term()->buffer[active_term()->buffer_size] = '\n';
            active_term()->buffer_size++;
            active_term()->ack = 1;
            putc('\n');
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

    }

    send_eoi(KEYBOARD_IRQ);
    enable_irq(KEYBOARD_IRQ);
}

/*
 * keyboard_read
 *   DESCRIPTION: This function reads inputs from the keyboard.
 *                This function blocks until ack is true.
 *                Here, ack true implies the following:
 *                1. The user pressed enter
 *                2. The command buffer is filled
 *   INPUTS: fd - File Descriptor (unused)
 *           buf - The destination to copy the input buffer contents to
 *           nbytes - The size of the buffer filled
 *   OUTPUTS: none
 *   RETURN VALUE: returns the number of bytes in the buffer
 *   SIDE EFFECTS: Flushes the buffer provided with the input buffer
 */
int
keyboard_read(int32_t fd, void* buf, int32_t nbytes)
{
    /* allow buffer filling */
    executing_term()->read_ack = 1;
    /* resetting flag at every read */
    executing_term()->ack = 0;
    /* spin until user presses Enter or the buffer has been filled */
    while(!executing_term()->ack);

    executing_term()->ack = 0;
    executing_term()->read_ack = 0;
    disable_irq(KEYBOARD_IRQ);
    uint32_t size;

    if(executing_term()->buffer_size < nbytes)
        size = executing_term()->buffer_size;
    else
        size = nbytes;

    memcpy(buf, executing_term()->buffer, size);
    memset(executing_term()->buffer, '\0', MAX_BUFFER_SIZE);
    executing_term()->buffer_size = 0;
    enable_irq(KEYBOARD_IRQ);

    return size;
}


/*
 * keyboard_write
 *   DESCRIPTION: This function does nothing as of now.
 *                It will be used to handle command bytes
 *                from kernel to keyboard eventually.
 *   INPUTS: fd - File Descriptor
 *           buf - The buffer to get data from
 *           nbytes - The number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int
keyboard_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return 0;
}


/*
 * keyboard_open
 *   DESCRIPTION: This function enables keyboard interrupt requests.
 *   INPUTS: filename (unused)
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: initializes the keyboard
 */
int
keyboard_open(const uint8_t* filename)
{
    keyboard_init();
    return 0;
}


/*
 * keyboard_close
 *   DESCRIPTION: Does nothing currently
 *   INPUTS: fd - File Descriptor (unused)
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int
keyboard_close(int32_t fd)
{
    return 0;
}


/*
 * check_modifier_keys
 *   DESCRIPTION: Sets the corresponding flags if any modifier keys are pressed
 *                or released - L/R-CTRL, L/R-SHIFT, CAPS_LOCK
 *   INPUTS: scan1, scan2 - the scan codes received from the keyboard
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if flags were changed, otherwise 0
 *   SIDE EFFECTS: changes the flags
 */
int32_t
check_modifier_keys(uint8_t scan1, uint8_t scan2)
{
    if (scan1 == CTRL_PRESSED)
    {
        l_ctrl = 1;
        return 1;
    }
    else if (scan1 == CTRL_RELEASED)
    {
        l_ctrl = 0;
        return 1;
    }

    if (scan1 == CTRL_PRESSED && scan2 == EXTENSION)
    {
        r_ctrl = 1;
        return 1;
    }
    else if (scan1 == CTRL_RELEASED && scan2 == EXTENSION)
    {
        r_ctrl = 0;
        return 1;
    }

    if (scan1 == ALT_PRESSED)
    {
        l_alt = 1;
        return 1;
    }
    else if (scan1 == ALT_RELEASED)
    {
        l_alt = 0;
        return 1;
    }

    if (scan1 == ALT_PRESSED && scan2 == EXTENSION)
    {
        r_alt = 1;
        return 1;
    }
    else if (scan1 == ALT_RELEASED && scan2 == EXTENSION)
    {
        r_alt = 0;
        return 1;
    }

    /* Check the status of Shift Keys */
    if (scan1 == L_SHIFT_PRESSED)
    {
        l_shift = 1;
        return 1;
    }
    else if (scan1 == R_SHIFT_PRESSED)
    {
        r_shift = 1;
        return 1;
    }
    else if (scan1 == L_SHIFT_RELEASED)
    {
        l_shift = 0;
        return 1;
    }
    else if (scan1 == R_SHIFT_RELEASED)
    {
        r_shift = 0;
        return 1;
    }

    /* Check the status of Caps Keys */
    if (scan1 == CAPS_PRESSED)
    {
        caps = !caps;
        return 1;
    }

    return 0;
}


/*
 * check_function_keys
 *   DESCRIPTION: Checks if an ALT-FX comnination is being pressed, and causes
 *                a terminal switch if it is valid
 *   INPUTS: scan1 - the keyboard scan value
 *   OUTPUTS: none
 *   RETURN VALUE: int32_t - 1 if successful, 0 if not detected
 *   SIDE EFFECTS: none
 */
int32_t
check_function_keys(uint8_t scan1)
{
    if(l_alt || r_alt)
    {
        if (scan1 >= FUNCTION_1 && scan1 <= FUNCTION_3)
        {
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            /* switch the terminal */
            switch_active_terminal(scan1 - FUNCTION_1);

            return 1;
        }
    }

    /* will not come here if terminal was switched */
    return 0;
}


/*
 * check_control_codes
 *   DESCRIPTION: Updates the buffer if any control codes have been detected.
 *                Currently supports - CTRL-A, CTRL-C, CTRL-L
 *   INPUTS: scan1 - the scan code received from the keyboard
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if CTRL is pressed, 0 otherwise
 *   SIDE EFFECTS: modifies the buffer
 */
int32_t
check_control_codes(uint8_t scan1)
{
    if(l_ctrl || r_ctrl)
    {
        if(scan1 == SCAN_L)
        {
            memset(active_term()->buffer, '\0', MAX_BUFFER_SIZE);
            active_term()->buffer_size = 0;
            active_term()->buffer[active_term()->buffer_size] = CTRL_L;
            active_term()->ack = 1;
            active_term()->buffer_size = 1;
        }
        else if(scan1 == SCAN_A)
        {
            memset(active_term()->buffer, '\0', MAX_BUFFER_SIZE);
            active_term()->buffer_size = 0;
            active_term()->buffer[active_term()->buffer_size] = CTRL_A;
            active_term()->ack = 1;
            active_term()->buffer_size = 1;
        }
        else if(scan1 == SCAN_C)
        {
            memset(active_term()->buffer, '\0', MAX_BUFFER_SIZE);
            active_term()->buffer_size = 0;
            active_term()->buffer[active_term()->buffer_size] = CTRL_C;
            active_term()->ack = 1;
            active_term()->buffer_size = 1;
        }

        /* return 1 if any CTRL key is pressed */
        return 1;
    }

    return 0;
}


/*
 * print_character
 *   DESCRIPTION: Converts the given scan code to its corresponding ASCII based
 *                on the values of the modifier flags
 *   INPUTS: scan1 - the scan code received from the keyboard
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Puts the character in the input buffer and on the screen
 */
void
print_character(uint8_t scan1)
{
    /* The following implementation is to handle other keys and cases */
    uint8_t output = scan_code_1[0][scan1];

    if(caps ^ (l_shift || r_shift))
    {
        if(output >= 'a' && output <= 'z')
            output -= OFFSET_TO_UPPERCASE;
    }

    if(l_shift || r_shift)
    {
        if(!(output >= 'A' && output <= 'Z') &&
           !(output >= 'a' && output <= 'z'))
            output = scan_code_1[1][scan1];
    }

    if(scan1 == SCAN_TAB)
    {
        if (active_term()->read_ack)
        {
            active_term()->buffer[active_term()->buffer_size] = ' ';
            active_term()->buffer_size++;
        }
        putc(' ');
    }
    else if(output == 0 || scan1 == CURSOR_UP || scan1 == CURSOR_RIGHT ||
            scan1 == CURSOR_LEFT || scan1 == CURSOR_DOWN);
    else
    {
        if (active_term()->read_ack)
        {
            active_term()->buffer[active_term()->buffer_size] = output;
            active_term()->buffer_size++;
        }
        putc(output);
    }
}
