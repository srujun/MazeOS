/*
 * syscalls_asm.S - Assembly jump table for page tables
 */

#define ASM     1

.text

sys_retval:
    .long 0x0

syscall_jmp_table:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, \
    set_handler, sigreturn

.global syscall_handler
syscall_handler:
    # save all registers and EFLAGS
    pushl    %es
    pushl    %ds
    pushl    %ebx
    pushl    %ecx
    pushl    %edx
    pushl    %esi
    pushl    %edi
    pushl    %ebp
    pushfl

    movw     $0x18, %di
    movw     %di, %ds

    # pushl    %ebp
    # pushl    %edi
    # pushl    %esi

    # store arguments to the stack
    pushl    %edx
    pushl    %ecx
    pushl    %ebx

    # sysnum has to be >= 1 and <= 10
    cmpl     $1, %eax
    jb       invalid_syscall
    cmpl     $10, %eax
    ja       invalid_syscall

    call      *syscall_jmp_table(, %eax, 4)
    jmp      finish_syscall

invalid_syscall:
    movl     $-1, %eax

finish_syscall:
    addl     $12, %esp           # pop syscall args
    movl     %eax, sys_retval    # save the return value

    # restore all registers and EFLAGS
    popfl
    popl     %ebp
    popl     %edi
    popl     %esi
    popl     %edx
    popl     %ecx
    popl     %ebx
    popl     %ds
    popl     %es
    movl     sys_retval, %eax    # put return value back in eax
    iret
