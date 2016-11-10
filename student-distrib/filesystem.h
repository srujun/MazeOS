/* filesystem.h - Defines the Read-Only file system functions */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"
#include "lib.h"

#define FILENAME_SIZE          32
#define MAX_DATA_BLOCK_COUNT   63
#define ELF_HEADER_SIZE        _4B
#define ENTRYPOINT_OFFSET      24

#define MAX_OPEN_FILES         8

#define FILE_USE_MASK          0x1
#define FILE_TYPE_MASK         0x6

// File usage
#define FILE_IN_USE            1

// File types
#define RTC_FILE_TYPE          0
#define DIR_FILE_TYPE          1
#define NORMAL_FILE_TYPE       2

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

typedef struct file_ops {
    int32_t (*open) (const uint8_t *);
    int32_t (*close) (int32_t);
    int32_t (*read) (int32_t, void *, int32_t);
    int32_t (*write) (int32_t, const void *, int32_t);
} file_ops_t;

typedef struct file_desc {
    file_ops_t * file_ops;
    inode_t * inode;
    uint32_t pos;
    uint32_t flags;
} file_desc_t;

/* Externally visible functions */

void fs_init(void * start_addr, void * end_addr);
int32_t get_file_size(dentry_t * d);
inode_t* get_inode_ptr(uint32_t inode);
uint32_t get_inode_from_ptr(inode_t * inode_ptr);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t fs_open(const uint8_t* filename);
int32_t fs_close(int32_t fd);
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);

file_ops_t fs_ops = {fs_open, fs_close, fs_read, fs_write};

#endif /* FILESYSTEM_H */
