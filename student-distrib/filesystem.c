/* filesystem.c - Read-Only file system functions */

#include "filesystem.h"
#include "lib.h"
#include "process.h"

#define DENTRY_SIZE      64
#define BLOCK_SIZE       4096
#define TYPE_DIRECTORY   1

static fs_metadata_t metadata;
static void * fs_start_addr;
static void * fs_end_addr;

static uint32_t f_idx;

/*
 * fs_init
 *   DESCRIPTION: Initializes the filesystem by parsing the metadata
 *   INPUTS: start_addr - the memory address at which the filesystem data starts
 *           end_addr - the memory address at which the filesystem data ends
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Parses the Filesystem metadata
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
 * get_file_size
 *   DESCRIPTION: Gets the file size in bytes
 *   INPUTS: d - pointer to the file's dentry_t object
 *   OUTPUTS: none
 *   RETURN VALUE: file size in bytes
 *   SIDE EFFECTS: none
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
 * get_inode_ptr
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
inode_t *
get_inode_ptr(uint32_t inode)
{
    return (inode_t*)((inode + 1) * BLOCK_SIZE + fs_start_addr);
}

/*
 * get_inode_from_ptr
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
uint32_t
get_inode_from_ptr(inode_t * inode_ptr)
{
    return (uint32_t)((inode_ptr - fs_start_addr)/BLOCK_SIZE - 1);
}

/*
 * fs_open
 *   DESCRIPTION: Checks if the file exists in the filesystem
 *   INPUTS: filename - the name of the file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - file exists
 *                 -1 - the file does not exist
 *   SIDE EFFECTS: none
 */
int32_t
fs_open(const uint8_t* filename)
{
    dentry_t d;
    return read_dentry_by_name(filename, &d);
}


/*
 * fs_close
 *   DESCRIPTION: Currently does nothing
 *   INPUTS: fd - ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t
fs_close(int32_t fd)
{
    return 0;
}


/*
 * fs_read
 *   DESCRIPTION: Reads the bytes of a given file based on the file descriptor
 *   INPUTS: fd - pointer to a file_desc_t object,
 *           buf - the buffer to put the bytes into
 *           nbytes - the number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes actually read
 *   SIDE EFFECTS: increments f_idx
 */
int32_t
fs_read(int32_t fd, void* buf, int32_t nbytes)
{
    file_desc_t fd_file = get_pcb()->fds[fd];
    uint32_t inode = get_inode_from_ptr(fd_file.inode);

    /* read directory */
    if (fd_file.flags & FILE_TYPE_MASK == DIR_FILE_TYPE)
    {
        dentry_t d;
        if (!read_dentry_by_index(fd_file.pos, &d))
        {
            get_pcb()->fds[fd].pos++;
            if(nbytes <= FILENAME_SIZE)
            {
                memcpy(buf, d.filename, nbytes);
                return nbytes;
            }
            else
            {
                memcpy(buf, d.filename, FILENAME_SIZE + 1);
                return FILENAME_SIZE + 1;
            }
        }
        /* read_dentry_by_index failed */
        return 0;
    }
    else if(fd_file.flags & FILE_TYPE_MASK == NORMAL_FILE_TYPE)
    {
        int32_t bytes_read = read_data(inode, fd_file.pos, buf, nbytes);
        if(bytes_read == -1)
            return 0;

        get_pcb()->fds[fd].pos += bytes_read;
        return bytes_read;
    }
    /* Filetype is broken */
    return 0;
}


/*
 * fs_write
 *   DESCRIPTION: Unsupported
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t
fs_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return 0;
}


/*
 * read_dentry_by_name
 *   DESCRIPTION: Gets the dentry_t object of a file specified by its name
 *   INPUTS: fname - the file's name
 *           dentry - pointer to the dentry_t object to fill
 *   OUTPUTS: populates dentry
 *   RETURN VALUE: 0 - success
 *                 -1 - failed to get file
 *   SIDE EFFECTS: none
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

    /* Non-existant file, filename did not match any */
    return -1;
}


/*
 * read_dentry_by_index
 *   DESCRIPTION: Gets the dentry_t object of a file specified by its index in
 *                the filesystem
 *   INPUTS: index - the file's index
 *           dentry - pointer to the dentry_t object to fill
 *   OUTPUTS: populates dentry
 *   RETURN VALUE: 0 - success
 *                 -1 - failed to get file
 *   SIDE EFFECTS: none
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
 *   DESCRIPTION: Reads a given inode data starting from the given offset,
 *                until the given length. Stops reading at the end of file, if
 *                the given length is longer than where we can read till
 *   INPUTS: inode - the inode number of the file to read,
 *           offset - the offset in bytes from where to start reading,
 *           buf - the buffer to put the bytes into,
 *           length - the number of bytes to read
 *   OUTPUTS: the bytes read from the file
 *   RETURN VALUE: -1 - failed to read the file (invalid inode or block number)
 *                 otherwise, number of bytes read (>= 0)
 *   SIDE EFFECTS: none
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

    /* if offset is greater than length, we cant read */
    if (file_inode.length < offset)
        return 0;

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

    /* check for invalid data block */
    if (curr_data_block_ptr > fs_end_addr)
        return -1;

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

    /* we keep reading whole blocks until either:
       1. the number of bytes still to read is less than the block's size
       2. we have read all the bytes requested */
    while (bytes_read < length)
    {
        /* check for invalid data block */
        if (curr_data_block_ptr > fs_end_addr)
            return -1;

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
