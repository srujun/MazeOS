/*
 * keyboard.c
 * Definitions of the functions that initialize and handle
 * the keyboard interrupts
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "types.h"

/* flags keeps tracks of all the special keys */
static int buffer_size = 0;
static uint8_t buffer[MAX_BUFFER_SIZE];

static uint8_t l_shift;
static uint8_t r_shift;
static uint8_t caps;
static uint8_t l_ctrl;
static uint8_t r_ctrl;

volatile uint8_t ack;

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
    memset(buffer, '\0', MAX_BUFFER_SIZE);

    buffer_size = 0;
    ack = 0;
    l_shift = 0;
    r_shift = 0;
    caps = 0;
    l_ctrl = 0;
    r_ctrl = 0;

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

    /* check modifier keys like SHIFT, CTRL, and CAPS LOCK */
    if (check_modifier_keys(c, c2))
    {
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }

    /* check if user has pressed any control code combinations */
    if (check_control_codes(c))
    {
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }

    /* Check if any button is being pressed */
    if (!(c & RELEASED_MASK))
    {
        /* handle backspace input */
        if(c == BACKSPACE)
        {
            if (buffer_size == 0)
            {
                /* don't do backspace if buffer is empty */
                send_eoi(KEYBOARD_IRQ);
                enable_irq(KEYBOARD_IRQ);
                return;
            }
            buffer_size--;
            buffer[buffer_size] = '\0';
            do_backspace();
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* handle enter key */
        if(c == ENTER_KEYCODE)
        {
            buffer[buffer_size] = '\n';
            buffer_size++;
            ack = 1;
            putc('\n');
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* check for buffer filled */
        if(buffer_size == MAX_BUFFER_SIZE - 1)
        {
            buffer[buffer_size] = '\0';
            ack = 1;
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* otherwise print the character */
        print_character(c);
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
    /* spin until user presses Enter or the buffer has been filled */
    while(!ack);

    ack = 0;
    disable_irq(KEYBOARD_IRQ);
    uint32_t size;

    if(buffer_size < nbytes)
        size = buffer_size;
    else
        size = nbytes;

    memcpy(buf, buffer, size);
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    buffer_size = 0;
    enable_irq(KEYBOARD_IRQ);

    return size;
}


/*
 * get_kb_buffer
 *   DESCRIPTION: This function copies the current buffer into the given pointer
 *                Acknowledges if control codes are present
 *   INPUTS: fd - File Descriptor (unused)
 *           buf - The buffer to write data to
 *           nbytes - The number of bytes to be copied
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
get_kb_buffer(void* buf)
{
    disable_irq(KEYBOARD_IRQ);

    memcpy(buf, buffer, MAX_BUFFER_SIZE);
    /* if control codes are present, we need to acknowledge it for handling
       them within the shell program */
    if (buffer[0] == CTRL_A || buffer[0] == CTRL_C || buffer[0] == CTRL_L)
    {
        memset(buffer, '\0', MAX_BUFFER_SIZE);
        buffer_size = 0;
        ack = 0;
    }

    enable_irq(KEYBOARD_IRQ);
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
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            buffer[buffer_size] = CTRL_L;
            ack = 1;
            buffer_size = 1;
        }
        else if(scan1 == SCAN_A)
        {
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            buffer[buffer_size] = CTRL_A;
            ack = 1;
            buffer_size = 1;
        }
        else if(scan1 == SCAN_C)
        {
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            buffer[buffer_size] = CTRL_C;
            ack = 1;
            buffer_size = 1;
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
        int i;
        for (i = 0; i < 4; i++)
        {
            buffer[buffer_size] = (' ');
            buffer_size++;
            putc(' ');
        }
    }
    else if(output == 0 || scan1 == CURSOR_UP || scan1 == CURSOR_RIGHT ||
            scan1 == CURSOR_LEFT || scan1 == CURSOR_DOWN);
    else
    {
        buffer[buffer_size] = output;
        buffer_size++;
        putc(output);
    }
}
