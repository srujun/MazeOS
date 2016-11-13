/*
 * syscalls.c - Defines function for syscalls infrastructure
 */

#include "syscalls.h"
#include "../paging.h"
#include "../process.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "../filesystem.h"
#include "../rtc.h"
#include "../keyboard.h"
#include "../terminal.h"

static file_ops_t fs_ops = {fs_open, fs_close, fs_read, fs_write};
static file_ops_t rtc_ops = {rtc_open, rtc_close, rtc_read, rtc_write};
static file_ops_t stdin_ops = {terminal_open, terminal_close,
                               terminal_read, NULL};
static file_ops_t stdout_ops = {terminal_open, terminal_close,
                                NULL, terminal_write};


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
    int i;
    pcb_t * pcb = get_pcb();

    /* close file descriptors */
    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        if ((pcb->fds[i].flags & FILE_USE_MASK) == FILE_IN_USE)
            pcb->fds[i].file_ops->close(i);
    }

    if (0 != free_pid(pcb->pid))
        printf("Should not have printed!\n");

    uint32_t k_esp, k_ebp;

    if (pcb->pid == 1) // Main process
    {
        k_ebp = k_esp = _8MB - _4B - (pcb->pid - 1) * _8KB;
        clear_setpos(0, 0);

        /* restore paging by mapping parent's page in the page directory */
        map_pde(pcb->pde_virt_addr, pcb->pde);
        flush_tlb();

        asm volatile (
            "movl %0, %%esp      \n\t"
            "movl %1, %%ebp      \n\t"
            :
            : "r" (k_esp), "r" (k_ebp)
            : "%esp", "%ebp"
        );
        execute((uint8_t *)"shell");
    }
    else // halting a child process
    {
        k_esp = pcb->parent->k_esp;
        k_ebp = pcb->parent->k_ebp;

        /* restore paging by mapping parent's page in the page directory */
        map_pde(pcb->parent->pde_virt_addr, pcb->parent->pde);
        flush_tlb();
    }

    /* restore parent data */
    tss.esp0 = k_esp;

    asm volatile (
        "movl %0, %%eax      \n\t"
        "movl %1, %%esp      \n\t"
        "movl %2, %%ebp      \n\t"
        "jmp BIG_FAT_RETURN"
        // "leave               \n\t"
        // "ret"
        :
        : "r" ((uint32_t) status), "r" (k_esp), "r" (k_ebp)
        : "%eax", "%esp", "%ebp"
    );

    /* should never happen */
    return 0;
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
    void* load_addr;
    uint32_t retval, copied, bytes_read;

    // get the first word in command -> filename
    uint8_t filename[FILENAME_SIZE];
    uint8_t args[ARGS_LENGTH];
    memset(filename, '\0', FILENAME_SIZE);
    memset(args, '\0', ARGS_LENGTH);
    
    uint16_t i, args_length = 0;

    /* get the filename */
    for(i = 0; command[i] != ' ' && command[i] != '\n' &&
               command[i] != '\0' && i < FILENAME_SIZE; i++)
    {
        filename[i] = command[i];
    }

    i++; // i points to ' '

    /* remove leading spaces */
    for(; command[i] == ' '; i++);

    /* get the args */
    for(; command[i] != '\n' && command[i] != '\0' &&
          args_length < ARGS_LENGTH; i++)
    {
        args[args_length] = command[i];
        args_length++;
    }

    /* strip the trailing spaces */
    for(--i; command[i] == ' '; i--)
        args_length--;

    /* check if filename is valid */
    dentry_t dentry;
    if (0 != read_dentry_by_name(filename, &dentry))
        return -1;

    /* check ELF header */
    if (ELF_HEADER != get_elf_header(dentry.inode))
        return -1;

    /* get executable start address */
    void * eip = get_elf_entrypoint(dentry.inode);

    pcb_t pcb;
    pcb.pid = get_available_pid();
    if (pcb.pid < 1 || pcb.pid > MAX_PROCESSES)
    {
        int8_t err[] = "Max processes reached\n";
        terminal_write(STDOUT, err, strlen(err));
        return 0;
    }

    /* put args into pcb */
    memcpy(pcb.args, args, args_length);

    /* create 4MB page for new process (either at 8MB or 12MB physical) */
    pcb.pde_virt_addr = _128MB;

    /* clear the pcb's pde entry and set the bits */
    memset(&(pcb.pde), 0, sizeof(pde_t));
    pcb.pde.present = 1;
    pcb.pde.read_write = 1;
    pcb.pde.user_supervisor = 1;
    pcb.pde.writethrough = 0;
    pcb.pde.cache_disabled = 0;
    pcb.pde.accessed = 0;
    pcb.pde.dirty = 0;
    pcb.pde.page_size = 1;
    pcb.pde.global = 0;
    pcb.pde.available = 0;
    pcb.pde.attr_index = 0;
    pcb.pde.reserved = 0;
    pcb.pde.base_addr = (_8MB + _4MB * (pcb.pid - 1)) >> BASE_ADDR_4MB_OFFSET;

    /* initialize the user stack base and pointer */
    pcb.ebp = pcb.esp = _128MB + _4MB - _4B;

    /* initialize the kernel stack pointer */
    pcb.k_ebp = pcb.k_esp = _8MB - _4B - (pcb.pid - 1) * _8KB;

    /* load the parent pcb pointer */
    if (pcb.pid == 1) // we are the first process
        pcb.parent = NULL;
    else
    {
        pcb.parent = get_pcb();
        /* IMPORTANT: store current esp value in pcb of parent */
        asm volatile (
            "movl %%esp, %0     \n\t"
            "movl %%ebp, %1     \n\t"
            : "=r" (pcb.parent->k_esp), "=r" (pcb.parent->k_ebp)
        );
    }

    /* map this page in the page directory */
    map_pde(pcb.pde_virt_addr, pcb.pde);
    flush_tlb();

    /* load the program into the 128MB virtual address with offset 0x48000 */
    load_addr = (void*)(_128MB + IMAGE_LOAD_OFFSET);
    copied = 0;
    bytes_read = 0;
    
    while (0 != (copied = read_data(dentry.inode, bytes_read, load_addr, _1KB)))
    {
        load_addr += copied;
        bytes_read += copied;
    }

    /* initialize the file descriptor array */
    pcb.fds[STDIN].file_ops = &stdin_ops;
    pcb.fds[STDIN].inode = NULL;
    pcb.fds[STDIN].pos = 0;
    pcb.fds[STDIN].flags = FILE_IN_USE;

    pcb.fds[STDOUT].file_ops = &stdout_ops;
    pcb.fds[STDOUT].inode = NULL;
    pcb.fds[STDOUT].pos = 0;
    pcb.fds[STDOUT].flags = FILE_IN_USE;

    for (i = 2; i < MAX_OPEN_FILES; i++)
    {
        pcb.fds[i].file_ops = NULL;
        pcb.fds[i].inode = NULL;
        pcb.fds[i].pos = 0;
        pcb.fds[i].flags = 0;
    }

    /* copy the new pcb to the new kernel stack */
    memcpy((void*)(pcb.k_esp & ESP_PCB_MASK), &pcb, sizeof(pcb_t));

    /* context switch -> write TSS values */
    tss.esp0 = pcb.k_esp; //_8MB - (pcb.pid - 1) * _8KB;
    tss.ss0 = KERNEL_DS;
    // tss.ss0 does not need to be updated (remains KERNEL_DS)

    /* create IRET context */
    asm volatile (
        // disable interrupts for critical section
        // ".global halt_return \n\t"
        "cli                 \n\t"

        // edit the segment registers
        "movl %0, %%ds       \n\t"
        "movl %0, %%es       \n\t"
        "movl %0, %%fs       \n\t"
        "movl %0, %%gs       \n\t"
    
        // push in order SS, ESP, EFLAGS, CS, EIP
        "pushl %0            \n\t"
        "pushl %1            \n\t"
        "pushfl              \n\t"
        // change IF so that interrupts become enabled when we reach userland
        "orl $0x200, (%%esp) \n\t"
        "pushl %2            \n\t"
        "pushl %3            \n\t"
        "iret"
        :
        : "r" (USER_DS), "r" (pcb.esp), "r" (USER_CS), "r" (eip)
    );
    asm volatile (
        "BIG_FAT_RETURN:     \n\t"
        "mov %%eax, %0"
        : "=r" (retval)
    );

    return retval;
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
    if((fd_file.flags & FILE_USE_MASK) == FILE_IN_USE)
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
    if((fd_file.flags & FILE_USE_MASK) == FILE_IN_USE)
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
        while((pcb->fds[i].flags & FILE_USE_MASK) != FILE_IN_USE)
            i++;
        if (i >= MAX_OPEN_FILES)
            /* no available file descriptor */
            return -1;

        file_desc_t fd;
        fd.pos = 0;
        fd.flags = ((d.filetype << 1) & FILE_TYPE_MASK) | FILE_IN_USE;

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

    return -1;
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
    if ((pcb->fds[fd].flags & FILE_USE_MASK) == FILE_IN_USE)
    {
        if (0 != pcb->fds[fd].file_ops->close(fd))
        {
            pcb->fds[fd].file_ops = NULL;
            return 0;
        }
    }

    return -1;
}


int32_t getargs(uint8_t * buf, int32_t nbytes)
{
    return 0;
}


int32_t vidmap(uint8_t** screen_start)
{
    return 0;
}


int32_t set_handler(int32_t signum, void * handler)
{
    return 0;
}


int32_t sigreturn(void)
{
    return 0;
}
