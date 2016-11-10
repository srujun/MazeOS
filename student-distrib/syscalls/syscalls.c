/*
 * syscalls.c - Defines function for syscalls infrastructure
 */

#include "syscalls.h"
#include "../process.h"
#include "../lib.h"
#include "../filesystem.h"
#include "../rtc.h"


/*
 * halt
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
halt(uint8_t status)
{
    
}


/*
 * execute
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
execute(const uint8_t * command)
{
    
}


/*
 * read
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
read(int32_t fd, void * buf, int32_t nbytes)
{
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if(buf == NULL)
        return -1;

    file_desc_t fd_file = get_pcb()->fds[fd];

    /* read only if file is in use */
    if(fd_file.flags & FILE_USE_MASK == FILE_IN_USE)
    {
        if (fd_file.file_ops->read != NULL)
            return fd_file.file_ops->read(fd, buf, nbytes);
    }
    return -1;
}


/*
 * write
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
write(int32_t fd, const void * buf, int32_t nbytes)
{
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if(buf == NULL)
        return -1;

    file_desc_t fd_file = get_pcb()->fds[fd];

    /* write only if file is in use */
    if(fd_file.flags & FILE_USE_MASK == FILE_IN_USE)
    {
        if (fd_file.file_ops->write != NULL)
            return fd_file.file_ops->write(fd, buf, nbytes);
    }
    return -1;
}


/*
 * open
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
open(const uint8_t * filename)
{
    dentry_t d;
    pcb_t * pcb;

    if (-1 != read_dentry_by_name(filename, &d))
    {
        pcb = get_pcb();
        int i = 2;
        /* find available fd */
        while(pcb->fds[i].flags & FILE_USE_MASK != FILE_IN_USE)
            i++;
        if (i >= MAX_OPEN_FILES)
            /* no available file descriptor */
            return -1;

        file_desc_t fd;
        fd.pos = 0;
        fd.flags = (d.filetype << 1) & FILE_TYPE_MASK;

        if (d.filetype == RTC_FILE_TYPE)
        {
            if (0 != rtc_open(filename))
                return -1;
            fd.inode = NULL;
            fd.file_ops = &rtc_ops;
            memcpy(pcb->fds + i, &fd, sizeof(file_desc_t));
            return i;
        }
        else if (d.filetype == DIR_FILE_TYPE)
        {
            if (0 != fs_open(filename))
                return -1;
            fd.inode = NULL;
            fd.file_ops = &fs_ops;
            memcpy(pcb->fds + i, &fd, sizeof(file_desc_t));

            return i;
        }
        else if (d.filetype == NORMAL_FILE_TYPE)
        {
            fd.inode = get_inode_ptr(d.inode);
            fd.file_ops = &fs_ops;
            memcpy(pcb->fds + i, &fd, sizeof(file_desc_t));
            return i;
        }
        else
            return -1;
    }
}


/*
 * close
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
close(int32_t fd)
{
    if (fd < 2 || fd >= MAX_OPEN_FILES)
        return -1;

    pcb_t * pcb = get_pcb();

    /* close only if file in use */
    if (pcb->fds[fd].flags & FILE_USE_MASK == FILE_IN_USE)
    {
        if (0 != pcb->fds[fd].file_ops->close())
        {
            pcb->fds[fd].file_ops = NULL;
            return 0;
        }
    }

    return -1;
}
