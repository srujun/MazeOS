/*
 * syscalls.h - Declares functions for syscalls infrastructure
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../types.h"
#include "../filesystem.h"

#define SYS_HALT         1
#define SYS_EXECUTE      2
#define SYS_READ         3
#define SYS_WRITE        4
#define SYS_OPEN         5
#define SYS_CLOSE        6
#define SYS_GETARGS      7
#define SYS_VIDMAP       8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN    10

typedef struct pcb {
    uint16_t pid;

    uint32_t esp;
    uint32_t ebp;
    uint32_t k_esp;

    uint32_t pde;
    uint16_t pde_idx;

    file_desc_t * fds[MAX_OPEN_FILES];

    pcb_t * parent;
} pcb_t;

/* External functions */
extern int32_t syscall_handler();

/* syscalls */
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t * command);
extern int32_t read(int32_t fd, void * buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t open(const uint8_t * filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t * buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void * handler);
extern int32_t sigreturn(void);

#endif
