/*
 * paging_asm.S - Assembly functions for Paging initialization
 */

#define ASM     1

#define BIT31_MASK  0x80000000
#define BIT4_MASK   0x00000010

.text

/*
 * load_page_directory
 *   DESCRIPTION: Loads the Page Directory Base Register (PDBR/CR3) with the
 *                given address of the page directory array
 *   INPUTS: Page Directory array address
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clobbers EAX
 */
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

/*
 * enable_paging
 *   DESCRIPTION: Enables 4 MB pages and paging, by setting bit 4 in CR4
 *                and bit 31 in CR0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clobbers EAX
 */
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
