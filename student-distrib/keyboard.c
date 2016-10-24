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
        '.', '/', 16, '*',
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
        '>', '?', 16, '*',
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
    disable_irq(KEYBOARD_IRQ);

    uint8_t c = inb(KEYBOARD_PORT);
    uint8_t c2 = inb(KEYBOARD_PORT);

    /* Check the status of Left Ctrl Key,
       TODO: Handle the Left and Right ctrl spamming */
    if (c == CTRL_PRESSED)
    {
        l_ctrl = 1;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    else if (c == CTRL_RELEASED)
    {
        l_ctrl = 0;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    
    if (c == CTRL_PRESSED && c2 == EXTENSION)
    {
        r_ctrl = 1;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    else if (c == CTRL_RELEASED && c2 == EXTENSION)
    {
        r_ctrl = 0;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    

    /* Check the status of Shift Keys */
    if (c == L_SHIFT_PRESSED)
    {
        l_shift = 1;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    else if (c == R_SHIFT_PRESSED)
    {
        r_shift = 1;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    else if (c == L_SHIFT_RELEASED)
    {
        l_shift = 0;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    else if (c == R_SHIFT_RELEASED)
    {
        r_shift = 0;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }
    

    /* Check the status of Caps Keys */
    if (c == CAPS_PRESSED)
    {
        caps = !caps;
        send_eoi(KEYBOARD_IRQ);
        enable_irq(KEYBOARD_IRQ);
        return;
    }

    if(l_ctrl || r_ctrl)
    {
        if(c == SCAN_L)
        {
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            /* TODO: other keys */
            buffer[buffer_size] = CTRL_L;
            ack = 1;
            buffer_size = 1;
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }
        else if(c == SCAN_A)
        {
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            /* TODO: other keys */
            buffer[buffer_size] = CTRL_A;
            ack = 1;
            buffer_size = 1;
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }
        else if(c == SCAN_C)
        {
            memset(buffer, '\0', MAX_BUFFER_SIZE);
            buffer_size = 0;
            /* TODO: other keys */
            buffer[buffer_size] = CTRL_C;
            ack = 1;
            buffer_size = 1;
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }
        else
        {
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }
    }

    /* Check if any button is being pressed */
    if (!(c & RELEASED_MASK))
    {
        if(c == BACKSPACE)
        {
            if (buffer_size == 0)
            {
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

        if(c == ENTER_KEYCODE)
        {
            buffer[buffer_size] = '\n';
            ack = 1;
            putc('\n');
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        if(buffer_size == MAX_BUFFER_SIZE - 1)
        {
            buffer[buffer_size] = '\0';
            ack = 1;
            send_eoi(KEYBOARD_IRQ);
            enable_irq(KEYBOARD_IRQ);
            return;
        }

        /* The following implementation is to handle other keys and cases */
        uint8_t output = scan_code_1[0][c];

        if(caps ^ (l_shift || r_shift))
        {
            if(output >= 'a' && output <= 'z')
                output -= OFFSET_TO_UPPERCASE;
        }

        if(l_shift || r_shift)
        {
            if(!(output >= 'A' && output <= 'Z') && !(output >= 'a' && output <= 'z'))
                output = scan_code_1[1][c];
        }
        if(c == SCAN_TAB)
        {
            buffer[buffer_size] = (' ');
            buffer_size++;
            buffer[buffer_size] = (' ');
            buffer_size++;
            buffer[buffer_size] = (' ');
            buffer_size++;
            buffer[buffer_size] = (' ');
            buffer_size++;
            putc(' ');
            putc(' ');
            putc(' ');
            putc(' ');
        }
        else if(output == 0 || c == CURSOR_UP || c == CURSOR_RIGHT ||
                c == CURSOR_LEFT || c == CURSOR_DOWN);
        else
        {
            buffer[buffer_size] = output;
            buffer_size++;
            putc(output);
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
 *   INPUTS: fd - File Descriptor
 *           buf - The destination to copy the input buffer contents to
 *           nbytes - The size of the buffer filled
 *   OUTPUTS: none
 *   RETURN VALUE: returns the number of bytes read
 *   SIDE EFFECTS: Flushes the buffer provided with the input buffer
 */
int
keyboard_read(int32_t fd, void* buf, int32_t nbytes)
{
    /* spin until user presses Enter or the buffer has been filled */
    while(!ack);
    ack = 0;
    disable_irq(KEYBOARD_IRQ);
    memcpy(buf, buffer, buffer_size);
    uint32_t size = buffer_size;
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    buffer_size = 0;
    enable_irq(KEYBOARD_IRQ);

    return size;
}


/*
 * TODO: docs
 */
void
get_kb_buffer(void* buf)
{
    disable_irq(KEYBOARD_IRQ);

    memcpy(buf, buffer, MAX_BUFFER_SIZE);
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
 *           buf - The buffer to write data to
 *           nbytes - The number of bytes to be copied
 *   OUTPUTS: none
 *   RETURN VALUE: 0 indicates success, -1 indicates failure
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
 *   INPUTS: Pointer to the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: 0 indicates success, -1 indicates failure
 *   SIDE EFFECTS: Initializes the keyboard.
 */
int
keyboard_open(const uint8_t* filename)
{
    keyboard_init();
    return 0;
}

/*
 * keyboard_close
 *   DESCRIPTION: This function handles close
 *   INPUTS: fd - File Descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0 indicates success, -1 indicates failure
 *   SIDE EFFECTS: Does nothing.
 */
int
keyboard_close(int32_t fd)
{
    return 0;
}
