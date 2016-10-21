/* filesystem.c - Read-Only file system functions */

#include "filesystem.h"
#include "lib.h"

#define DENTRY_SIZE      64
#define BLOCK_SIZE       4096

static fs_metadata_t metadata;
static void * fs_start_addr;
static void * fs_end_addr;

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

    uint32_t fs_size = (uint32_t)(fs_end_addr - fs_start_addr) / 1024;
    memcpy(&metadata, fs_start_addr, sizeof(fs_metadata_t));

    printf("Filesystem size = %dkB, ", fs_size);
    printf("num inodes = %d, num data block = %d\n", metadata.num_inodes, metadata.num_data_blocks);

    int i, j, k;
    // for (i = 0; i < metadata.num_dentries; i++)
    // {
    //     dentry_t file;
    //     read_dentry_by_index(i, &file);
    //     printf("Name: %s, type: %d, inode: %d\n", file.filename, file.filetype, file.inode);
    // }

    uint8_t frame0[200];
    uint8_t frame1[200];
    uint32_t bytes1 = read_data(63, 0, frame0, 200);
    uint32_t bytes2 = read_data(26, 0, frame1, 200);
    
    for (k = 0; k < 10000; k++)
    {
        for (i = 0; i < 5000000; i++)
        {
            if (i % 1000000 == 0)
            {
                clear_setpos(0, 5);
                for (j = 0; j < bytes1; j++)
                    putc(frame0[j]);
            }
            else if (i % 500000 == 0)
            {
                clear_setpos(0, 5);
                for (j = 0; j < bytes2; j++)
                    putc(frame1[j]);
            }
        }
    }
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