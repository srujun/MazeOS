/*
 * syscalls.c - Defines function for syscalls infrastructure
 */

#include "syscalls/syscalls.h"
#include "paging.h"
#include "process.h"
#include "x86/x86_desc.h"
#include "lib.h"
#include "filesystem.h"
#include "drivers/rtc.h"
#include "drivers/keyboard.h"
#include "drivers/terminal.h"

static file_ops_t fs_ops = {fs_open, fs_close, fs_read, fs_write};
static file_ops_t rtc_ops = {rtc_open, rtc_close, rtc_read, rtc_write};
static file_ops_t stdin_ops = {terminal_open, terminal_close,
                               terminal_read, NULL};
static file_ops_t stdout_ops = {terminal_open, terminal_close,
                                NULL, terminal_write};


/*
 * halt
 *   DESCRIPTION: Halts the given process and returns back to parent process
 *   INPUTS: status - The status returned from the user process
 *   OUTPUTS: none
 *   RETURN VALUE: never returns (jumps back to execute)
 *   SIDE EFFECTS: Moves the Stack pointers back to parent process
 */
int32_t
halt(uint8_t status)
{
    int i;
    int32_t retval;

    cli();

    pcb_t * pcb = get_pcb();

    if (pcb->retval == RETURN_EXCEPTION)
        retval = RETURN_EXCEPTION;
    else
        retval = status;

    /* close file descriptors */
    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        if ((pcb->fds[i].flags & FILE_USE_MASK) == FILE_IN_USE)
            pcb->fds[i].file_ops->close(i);
    }

    if (0 != free_pid(pcb->pid))
        printf("Should not have printed!\n");

    uint32_t k_esp, esp0;

    /* unmap video memory if previously mapped */
    if (pcb->vidmem_virt_addr != 0)
        free_user_video_mem(pcb->vidmem_virt_addr);

    active_term()->num_procs--;
    active_term()->child_procs[active_term()->num_procs] = NULL;

    if (pcb->parent == NULL) // Main process
    {
        esp0 = k_esp = _8MB - _4B - (pcb->pid - 1) * _8KB;
        clear_setpos(0, 0);

        /* restart shell */
        execute((uint8_t *)"shell");
    }
    else // halting a child process
    {
        k_esp = pcb->parent->k_esp;
        esp0 = pcb->parent->esp0;

        /* restore paging by mapping parent's page in the page directory */
        map_page_4MB(pcb->parent->pde_virt_addr, pcb->parent->pde);
    }

    /* restore parent data */
    tss.esp0 = esp0;

    /* overwrite the stack pointers */
    asm volatile (
        "movl %0, %%eax      \n\t"
        "movl %1, %%esp      \n\t"
        "jmp BIG_FAT_RETURN"
        :
        : "r" (retval), "r" (k_esp)
        : "%eax", "%esp"
    );

    /* should never happen */
    return 0;
}


/*
 * execute
 *   DESCRIPTION: Creates a new process, loads the executable into remapped
 *                memory, creates the corresponding PCB in kernel memory, passes
 *                execution to the user program.
 *   INPUTS: command - the command to execute with arguments
 *   OUTPUTS: none
 *   RETURN VALUE: the status from the user process execution
 *   SIDE EFFECTS: Sets up PCB and the TSS, and changes the Privilege Level
 */
int32_t
execute(const uint8_t * command)
{
    uint32_t ret_kesp, ret_kebp;

    cli();

    asm volatile (
        "movl %%esp, %0     \n\t"
        "movl %%ebp, %1     \n\t"
        : "=r" (ret_kesp), "=r" (ret_kebp)
    );

    void* load_addr;
    uint32_t retval, copied, bytes_read;

    // get the first word in command -> filename
    uint8_t filename[FILENAME_SIZE];
    uint8_t args[ARGS_LENGTH];
    memset(filename, '\0', FILENAME_SIZE);
    memset(args, '\0', ARGS_LENGTH);

    uint16_t i, args_length = 0;

    /* check if command pointer is within userspace */
    if((uint32_t) command < _128MB || (uint32_t) command >= (_128MB + _4MB))
    {
        /* check if command pointer is within kernel space */
        if((uint32_t) command < _4MB || (uint32_t) command >= (_4MB + _4MB))
            return -1;
    }

    /* get the filename */
    for(i = 0; command[i] != ' ' && command[i] != '\n' &&
               command[i] != '\0' && i < FILENAME_SIZE; i++)
        filename[i] = command[i];

    /* remove leading spaces */
    for(; command[i] == ' '; i++);

    /* get the args */
    for(; command[i] != '\n' && command[i] != '\0' &&
          args_length < ARGS_LENGTH; i++)
    {
        args[args_length] = command[i];
        args_length++;
    }

    if (args_length != 0)
    {
        /* strip the trailing spaces */
        for(--i; command[i] == ' '; i--)
            args_length--;
        args[args_length] = '\0';
    }

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
    memset(&pcb, 0, sizeof(pcb_t));

    pcb.pid = get_available_pid();
    if (pcb.pid < 1 || pcb.pid > MAX_PROCESSES)
    {
        int8_t err[] = "Max processes reached\n";
        terminal_write(STDOUT, err, strlen(err));
        return 0;
    }

    // memset(pcb.args, '\0', ARGS_LENGTH);
    if (args_length != 0)
        /* put args into pcb */
        memcpy(pcb.args, args, args_length + 1);
    pcb.args_length = args_length;

    /* create 4MB page for new process (either at 8MB or 12MB physical) */
    pcb.pde_virt_addr = _128MB;
    pcb.vidmem_virt_addr = 0; // vidmem = NULL

    /* clear the pcb's pde entry and set the bits */
    // memset(&(pcb.pde), 0, sizeof(pde_4M_t));
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
    if (active_term()->num_procs == 0) // we are the first process in curr terminal
        pcb.parent = NULL;
    else
    {
        pcb.parent = get_pcb();
        /* IMPORTANT: store current esp value in pcb of parent */
        pcb.parent->k_esp = ret_kesp;
        pcb.parent->k_ebp = ret_kebp;
    }

    /* map this page in the page directory */
    map_page_4MB(pcb.pde_virt_addr, pcb.pde);

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

    /* copy the new pcb to the new kernel stack, also put the PCB pointer
       in the terminal_t struct */
    active_term()->child_procs[active_term()->num_procs] =
                        (void*)(pcb.k_esp & ESP_PCB_MASK);
    memcpy(active_term()->child_procs[active_term()->num_procs], &pcb, sizeof(pcb_t));
    active_term()->num_procs++;

    /* context switch -> write TSS values */
    tss.esp0 = pcb.k_esp;
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
 *   DESCRIPTION: Reads the bytes from the file descriptor
 *   INPUTS: fd - the file descriptor number
 *           buf - the buffer to put the bytes into
 *           nbytes - the number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes actually read
 *   SIDE EFFECTS: increments the offset in the file
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
 *   DESCRIPTION: Wrties the buffer to the file descriptor (stdin, stdout, rtc)
 *   INPUTS: fd - the file descriptor number
 *           buf - the buffer to write the bytes from
 *           nbytes - the number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes actually read
 *   SIDE EFFECTS: increments the offset in the file
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
 *   DESCRIPTION: Initializes a free file descriptor and returns the fd number
 *   INPUTS: The filename to open
 *   OUTPUTS: none
 *   RETURN VALUE: the fd number
 *   SIDE EFFECTS: Marks the file descriptor as in use
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
        while((pcb->fds[i].flags & FILE_USE_MASK) == FILE_IN_USE)
        {
            i++;
            if (i >= MAX_OPEN_FILES)
                /* no available file descriptor */
                return -1;
        }

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
 *   DESCRIPTION: Closes the given file descriptor
 *   INPUTS: The file descriptor number
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - successful
 *                 -1 - otherwise
 *   SIDE EFFECTS: Marks the given file descriptor as not in use
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
        if (0 == pcb->fds[fd].file_ops->close(fd))
        {
            pcb->fds[fd].flags = FILE_NOT_IN_USE;
            pcb->fds[fd].file_ops = NULL;
            return 0;
        }
    }

    return -1;
}


/*
 * getargs
 *   DESCRIPTION: Copies the supplied arguments into the given buffer
 *   INPUTS: buf - the buffer to copy the args into
 *           nbytes - the number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: -1 - if args do not fit into the buffer
 *                 0 - otherwise
 *   SIDE EFFECTS: none
 */
int32_t
getargs(uint8_t * buf, int32_t nbytes)
{
    /* check if buffer is within userspace memory */
    if((uint32_t) buf < _128MB || (uint32_t) buf >= (_128MB + _4MB))
        return -1;

    pcb_t * pcb = get_pcb();

    /* check if we have been given enough space to fit the whole args string */
    if (nbytes < pcb->args_length + 1)
        return -1;

    memcpy(buf, pcb->args, pcb->args_length + 1);
    return 0;
}


/*
 * vidmap
 *   DESCRIPTION: Maps a 4KB user-level memory page to the video memory and
 *                writes the address to the given pointer
 *   INPUTS: screen_start - the pointer to modify
 *   OUTPUTS: screen_start
 *   RETURN VALUE: 0 - successful
 *                 -1 - the given pointer is not owned by the user process
 *   SIDE EFFECTS: creates a new page mapping
 */
int32_t
vidmap(uint8_t** screen_start)
{
    /* check if screen start is within userspace memory */
    if((uint32_t) screen_start < _128MB ||
       (uint32_t) screen_start >= (_128MB + _4MB))
        return -1;

    pte_t pte;
    memset(&(pte), 0, sizeof(pte_t));

    /* create the video memory page with user access */
    pte.present = 1;
    pte.read_write = 1;
    pte.user_supervisor = 1;
    pte.writethrough = 0;
    pte.cache_disabled = 0;
    pte.accessed = 0;
    pte.attr_index = 0;
    pte.dirty = 0;
    pte.global = 0;
    pte.available = 0;
    pte.base_addr = VIDEO_MEM_INDEX;

    map_user_video_mem(USER_VIDEO_MEM_ADDR, pte);
    get_pcb()->vidmem_virt_addr = USER_VIDEO_MEM_ADDR;
    get_pcb()->vidmem_pte = pte;

    *screen_start = (uint8_t*)(USER_VIDEO_MEM_ADDR);

    return 0;
}


/*
 * set_handler
 *   DESCRIPTION: Unsupported
 *   INPUTS: signum, handler
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - unsupported
 *   SIDE EFFECTS: none
 */
int32_t
set_handler(int32_t signum, void * handler)
{
    return 0;
}


/*
 * sigreturn
 *   DESCRIPTION: Unsupported
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - unsupported
 *   SIDE EFFECTS: none
 */
int32_t
sigreturn(void)
{
    return 0;
}
