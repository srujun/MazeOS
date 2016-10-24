/* filesystem.h - Defines the Read-Only file system functions */
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define FILENAME_SIZE         32
#define MAX_DATA_BLOCK_COUNT  63

typedef struct dentry {
    char filename[FILENAME_SIZE + 1];
    uint32_t filetype;
    uint32_t inode;
} dentry_t;

typedef struct fs_metadata {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
} fs_metadata_t;

typedef struct inode {
    uint32_t length;
    uint32_t data_blocks[MAX_DATA_BLOCK_COUNT];
} inode_t;

/* Temporary for checkpoint 2 */
typedef struct fs_desc {
    int32_t index;
    uint8_t filename[FILENAME_SIZE + 1];
} fs_desc_t;

/* Externally visible functions */

void fs_init(void * start_addr, void * end_addr);
int32_t get_file_size(dentry_t * d);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t fs_open(const uint8_t* filename);
int32_t fs_close(int32_t fd);
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);

#endif /* FILESYSTEM_H */
