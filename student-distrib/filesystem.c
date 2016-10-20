/* filesystem.c - Read-Only file system functions */

#include "filesystem.h"
#include "lib.h"

/*
 * fs_init
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
void
fs_init(void * start_addr, void * end_addr)
{
    uint32_t fs_size = 10;// (uint32_t)(end_addr - start_addr);
    printf("Filesystem size = %dB\n", fs_size);
    // fs_metadata_t metadata;
    // memcpy(&metadata, )
}
