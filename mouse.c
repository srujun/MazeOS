/*
 * mouse.c
 * Definitions of the functions that initialize and handle
 * the mouse interrupts
 */

#include "drivers/rtc.h"
#include "x86/i8259.h"
#include "lib.h"
#include "drivers/mouse.h"
#include "../types.h"

/*
 * mouse_init
 *   DESCRIPTION:
 *   INPUTS: fd - File Descriptor
 *           buf - The buffer to get data from
 *           nbytes - The number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
void
mouse_init(){

}

/*
 * mouse_interrupt_handler
 *   DESCRIPTION:
 *   INPUTS: fd - File Descriptor
 *           buf - The buffer to get data from
 *           nbytes - The number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
void
mouse_interrupt_handler(){

}

/*
 * mouse_read
 *   DESCRIPTION:
 *   INPUTS: fd - File Descriptor
 *           buf - The buffer to get data from
 *           nbytes - The number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
void
mouse_read(int32_t fd, void* buf, int32_t nbytes)
{

}

/*
 * mouse_write
 *   DESCRIPTION:
 *   INPUTS: fd - File Descriptor
 *           buf - The buffer to get data from
 *           nbytes - The number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int
mouse_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return 0;
}

/*
 * mouse_open
 *   DESCRIPTION:
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int
mouse_open(const uint8_t* filename)
{
    return 0;
}

/*
 * mouse_close
 *   DESCRIPTION:
 *   INPUTS: fd - File Descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int
mouse_close(int32_t fd)
{
    return 0;
}
