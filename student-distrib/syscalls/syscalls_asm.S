/*
 * syscalls_asm.S - Assembly jump table for page tables
 */

#define ASM     1

.text

syscall_jmp_table:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, \
    set_handler, sigreturn


store_esp0:
    pushl    %ebx
    pushl    %ecx
    movl     %esp, %ebx
    andl     $0xFFFFE000, %ebx
    movl     %esp, %ecx
    addl     $8, %ecx
    movl     %ecx, (%ebx)
    popl     %ecx
    popl     %ebx
    jmp      syscall_handler_continue


.global syscall_handler
syscall_handler:
    # save the esp0 value in this process' PCB if execute has been called
    cmpl $2, %eax
    je   store_esp0
syscall_handler_continue:
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
    iret
