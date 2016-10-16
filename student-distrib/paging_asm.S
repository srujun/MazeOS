#define ASM     1

#define BIT31_MASK  0x80000000
#define BIT4_MASK   0x00000010

.text

# TODO: doc!
.globl load_page_directory
load_page_directory:
    # set up the stack
    pushl %ebp
    movl %esp, %ebp

    # load cr3 register (PDBR)
    movl 8(%esp), %eax
    movl %eax, %cr3

    # tear down the stack
    leave
    ret

.globl enable_paging
enable_paging:
    # set up the stack
    pushl %ebp
    movl %esp, %ebp

    # set bit 4 in CR4 register (enables 4MB pages)
    movl %cr4, %eax
    orl $BIT4_MASK, %eax
    movl %eax, %cr4

    # set bit 31 in CR0 register (enables paging)
    movl %cr0, %eax
    orl $BIT31_MASK, %eax
    movl %eax, %cr0

    # tear down the stack
    leave
    ret
