/* filesystem.h - Defines the Read-Only file system functions */
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

typedef struct fs_file {
    char name[32];
    uint32_t type;
    uint32_t inode_num;
} fs_file_t;

typedef struct fs_metadata {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
} fs_metadata_t;

/* Externally visible functions */

void fs_init(void * start_addr, void * end_addr);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif /* FILESYSTEM_H */
