/* filesystem.c - Read-Only file system functions */

#include "filesystem.h"
#include "lib.h"

#define DENTRY_SIZE      64
#define BLOCK_SIZE       4096
#define TYPE_DIRECTORY   1

static fs_metadata_t metadata;
static void * fs_start_addr;
static void * fs_end_addr;

/* Temporary for checkpoint 2 */
// static fs_desc_t fds[];
static uint32_t f_idx;

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
    fs_start_addr = start_addr;
    fs_end_addr   = end_addr;
    f_idx = 0;

    memcpy(&metadata, fs_start_addr, sizeof(fs_metadata_t));
}


/*
 * TODO
 */
int32_t
get_file_size(dentry_t * d)
{
    uint32_t inode = d->inode;
    inode_t file_inode;

    /* compute byte length of the file */
    void * inode_block_ptr = (inode + 1) * BLOCK_SIZE + fs_start_addr;
    memcpy(&(file_inode.length), inode_block_ptr, sizeof(uint32_t));
    return file_inode.length;
}


/*
 * TODO: check if the file exists
 */
int32_t fs_open(const uint8_t* filename)
{
    uint32_t suc;
    dentry_t d;
    suc = read_dentry_by_name(filename, &d);
    /* if (d.filetype == 1 && f_idx >= metadata.num_dentries)
        f_idx = 0;
        return -1; */
    return suc;
}


/*
 * TODO
 */
int32_t fs_close(int32_t fd)
{
    return 0;
}


/*
 * For now, fd is either a pointer to an index or a filename array
 */
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes)
{
    void * fdp = (void *) fd;
    fs_desc_t fd_file;

    memset(fd_file.filename, '\0', FILENAME_SIZE + 1);
    memcpy(&fd_file, fdp, sizeof(fs_desc_t));

    if (fd_file.index >= 0)
    {
        if (fd_file.index >= metadata.num_dentries)
            return 0;

        /* we have to read from index */
        dentry_t d;
        if (!read_dentry_by_index(fd_file.index, &d))
        {
            int32_t bytes_read = read_data(d.inode, 0, buf, nbytes);
            if(bytes_read == -1)
                return 0;

            return bytes_read;
        }
        /* read_dentry_by_index failed */
        return 0;
    }
    else
    {
        /* we have to read by filename */
        dentry_t d;
        int32_t suc = read_dentry_by_name(fd_file.filename, &d);

        /* directory by index */
        if (!suc && d.filetype == TYPE_DIRECTORY)
        {
            dentry_t f;
            if (!read_dentry_by_index(f_idx, &f))
            {
                f_idx++;
                int32_t bytes_read = read_data(f.inode, 0, buf, nbytes);
                if(bytes_read == -1)
                    return 0;

                return bytes_read;
                // memcpy(buf, f.filename, FILENAME_SIZE + 1);
                // int32_t len = strlen(f.filename);
                // f.filename[len] = '\0';
                // if (f.filetype == 2)
                //     return get_file_size(&f);
                // else
                //     return 1;
            }
            /* read_dentry_by_index failed */
            return 0;
        }
        /* read file contents, input was NOT directory */
        else if (!suc)
        {
            int32_t bytes_read = read_data(d.inode, 0, buf, nbytes);
            if(bytes_read == -1)
                return 0;

            return bytes_read;
        }
        /* read_dentry_by_index failed */
        return 0;
    }

    return 0;
}


/*
 *
 */
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return 0;
}


/*
 * read_dentry_by_name
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    int i;
    uint8_t dentry_name[FILENAME_SIZE];
    memset(dentry_name, '\0', FILENAME_SIZE);
    void * start = DENTRY_SIZE + fs_start_addr;

    /* Keep looping till we see a file in the current directory that matches
       the name of the requested file */
    for (i = 0; i < metadata.num_dentries; i++)
    {
        /* Get the current dentry's filename */
        memcpy(dentry_name, start, FILENAME_SIZE);

        /* check if the filenames match */
        if (!strncmp((int8_t *)fname, (int8_t *)dentry_name, FILENAME_SIZE))
        {
            /* Copy the dentry file name */
            memset(dentry->filename, '\0', FILENAME_SIZE + 1);
            memcpy(dentry->filename, start, FILENAME_SIZE);
            dentry->filename[FILENAME_SIZE] = '\0';

            /* Copy the dentry file type */
            start += FILENAME_SIZE;
            memcpy(&(dentry->filetype), start, sizeof(uint32_t));

            /* Copy the dentry inode */
            start += sizeof(uint32_t);
            memcpy(&(dentry->inode), start, sizeof(uint32_t));

            return 0;
        }

        start += DENTRY_SIZE;
    }

    /* Non-existant file or invalid index */
    return -1;
}


/*
 * read_dentry_by_index
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    if (index >= metadata.num_dentries)
    {
        /* Non-existant file or invalid index */
        return -1;
    }

    void * start = (index + 1) * DENTRY_SIZE + fs_start_addr;

    /* Copy the dentry file name */
    memcpy(dentry->filename, start, FILENAME_SIZE);
    dentry->filename[FILENAME_SIZE] = '\0';

    /* Copy the dentry file type */
    start += FILENAME_SIZE;
    memcpy(&(dentry->filetype), start, sizeof(uint32_t));

    /* Copy the dentry inode */
    start += sizeof(uint32_t);
    memcpy(&(dentry->inode), start, sizeof(uint32_t));

    return 0;
}


/*
 * read_data
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t
read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    if (inode >= metadata.num_inodes)
        /* invalid inode specified */
        return -1;

    int i;
    inode_t file_inode;

    /* compute byte length of the file */
    void * inode_block_ptr = (inode + 1) * BLOCK_SIZE + fs_start_addr;
    memcpy(&(file_inode.length), inode_block_ptr, sizeof(uint32_t));

    /* curb read length if we are being asked to read past the end of file */
    if (length > file_inode.length - offset)
        length = file_inode.length - offset;

    /* move inode_block_ptr to the data block number segment */
    inode_block_ptr += sizeof(uint32_t);

    /* Initialize the the data block num array in the inode_t object */
    for (i = 0; i < (file_inode.length / BLOCK_SIZE) + 1; i++)
    {
        memcpy(file_inode.data_blocks + i, inode_block_ptr,
               sizeof(uint32_t));
        inode_block_ptr += sizeof(uint32_t);
    }

    /* the current data block number we are reading from */
    uint32_t curr_data_block = offset / BLOCK_SIZE;
    /* keep track of how many bytes we have read so far */
    uint32_t bytes_read = 0;

    /* If we have to start reading from past the end of the file,
       return 0 (we read no bytes) */
    if (curr_data_block > file_inode.length / BLOCK_SIZE)
        return 0;

    /* curr_data_block_ptr points to:
       1. the start of the data block from which we have to read
       2. the specifc address until which we have read so far */
    void * curr_data_block_ptr =
        (file_inode.data_blocks[curr_data_block] + metadata.num_inodes + 1) *
        BLOCK_SIZE + fs_start_addr;

    void * curr_block_end = curr_data_block_ptr + BLOCK_SIZE;

    /* move to offset within current block */
    curr_data_block_ptr += offset % BLOCK_SIZE;
    /* remaining bytes to read in current data block */
    uint32_t rem_bytes = (uint32_t)(curr_block_end - curr_data_block_ptr);

    if (length < rem_bytes)
    {
        /* all the data to be read is in this block */
        memcpy(buf, curr_data_block_ptr, length);
        return length;
    }

    /* read the rest of the first data block */
    memcpy(buf, curr_data_block_ptr, rem_bytes);
    bytes_read += rem_bytes;
    /* increment buf by how much we read */
    buf += rem_bytes;

    /* move on to the next data block */
    curr_data_block++;
    curr_data_block_ptr =
        (file_inode.data_blocks[curr_data_block] + metadata.num_inodes + 1) *
        BLOCK_SIZE + fs_start_addr;

    /* we keep reading whole blocks until:
       1.  */
    while (bytes_read < length)
    {
        rem_bytes = length - bytes_read;
        if (rem_bytes < BLOCK_SIZE)
        {
            /* we are reading the last data block */
            memcpy(buf, curr_data_block_ptr, rem_bytes);
            bytes_read += rem_bytes;
            rem_bytes = 0;
            break;
        }
        else
        {
            /* we read the entire data block */
            memcpy(buf, curr_data_block_ptr, BLOCK_SIZE);
            bytes_read += BLOCK_SIZE;
            buf += BLOCK_SIZE;
        }
        /* move on to the next data block */
        curr_data_block++;
        curr_data_block_ptr =
            (file_inode.data_blocks[curr_data_block] + metadata.num_inodes + 1) *
            BLOCK_SIZE + fs_start_addr;
    }

    return bytes_read;
}
