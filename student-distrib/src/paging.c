/*
 * paging.c - Defines function for Paging initialzation
 */

#include "paging.h"
#include "lib.h"

#define SHIFT_4MB          22
#define SHIFT_4KB          12
#define MASK_10_BITS       0x3FF


/* Arrays to hold the Page Directory and the table for the first 4MB section */
static uint32_t page_directory[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));
static uint32_t first_4MB_table[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));
static uint32_t user_4MB_table[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));
static uint32_t backup_vidmem_table[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));


/*
 * init_paging
 *   DESCRIPTION: Initializes the page directory array and the first 4MB page
 *                table, and calls functions to set up the Control Registers
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables paging
 */
void
init_paging(void)
{
    int i;

    /* Initialize all PDE's to be NOT present, Read/Write, Supervisor */
    pde_4M_t default_pde;
    memset(&(default_pde), 0, sizeof(pde_4M_t));
    default_pde.present = 0;
    default_pde.read_write = 1;
    default_pde.user_supervisor = 0;

    for (i = 0; i < PAGE_COUNT; i++)
        memcpy(&page_directory[i], &default_pde, sizeof(pde_4M_t));

    /* Initialize the 4KB PTE's for the first 4MB table in physical memory,
       to NOT present, Read/Write, Supervisor */
    pte_t init_pte;
    memset(&(init_pte), 0, sizeof(pte_t));
    init_pte.present = 0;
    init_pte.read_write = 1;
    init_pte.user_supervisor = 0;

    for (i = 0; i < PAGE_COUNT; i++)
    {
        init_pte.base_addr = i;
        memcpy(&first_4MB_table[i], &init_pte, sizeof(pte_t));
    }

    /* Initialize the 4KB PTE's for the user 4KB page table */
    pte_t user_pte;
    memset(&(user_pte), 0, sizeof(pte_t));
    user_pte.present = 0;
    user_pte.read_write = 1;
    user_pte.user_supervisor = 1;

    for (i = 0; i < PAGE_COUNT; i++)
        memcpy(&user_4MB_table[i], &user_pte, sizeof(pte_t));

    /* Initialize video memory pages (32KB) starting at 0xB8000,
       to present, Read/Write, Supervisor */
    pte_t video_mem_pte;
    memset(&(video_mem_pte), 0, sizeof(pte_t));
    video_mem_pte.present = 1;
    video_mem_pte.read_write = 1;
    video_mem_pte.user_supervisor = 0;

    for (i = 0; i < VIDEO_MEM_PG_COUNT; i++)
    {
        video_mem_pte.base_addr = VIDEO_MEM_INDEX + i;
        memcpy(&first_4MB_table[VIDEO_MEM_INDEX + i],
               &video_mem_pte, sizeof(pte_t));
    }

    /* First PDE should point to first_4MB_table, set to present, Read/Write,
       and User access */
    pde_4K_t first_pde;
    memset(&(first_pde), 0, sizeof(pde_4K_t));
    first_pde.present = 1;
    first_pde.read_write = 1;
    first_pde.user_supervisor = 1;
    first_pde.base_addr = ((uint32_t) first_4MB_table) >> SHIFT_4KB;
    memcpy(&page_directory[0], &first_pde, sizeof(pde_4K_t));

    /* 4MB for Kernel Space, set to present, Read/Write, Supervisor */
    pde_4M_t kernel_pde;
    memset(&(kernel_pde), 0, sizeof(pde_4M_t));
    kernel_pde.present = 1;
    kernel_pde.read_write = 1;
    kernel_pde.user_supervisor = 0;
    kernel_pde.page_size = 1;
    kernel_pde.base_addr = KERNEL_MEM_START >> SHIFT_4MB;
    memcpy(&page_directory[1], &kernel_pde, sizeof(pde_4M_t));

    /* give the page_directory pointer to CR3 */
    load_page_directory((uint32_t) page_directory);
    /* call the enabler */
    enable_paging();
}


/*
 * map_page_4MB
 *   DESCRIPTION: Maps the given virtual address to the entry specified by
 *                the given Page Directory Entry. Creates a 4MB page.
 *   INPUTS: vir_addr - the virtual address within the page to map
 *           pde - the entry to put into the Page Directory for this page
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes the x86 TLBs
 */
void
map_page_4MB(uint32_t vir_addr, pde_4M_t pde)
{
    uint32_t pde_bytes;
    memcpy(&pde_bytes, &pde, sizeof(pde_4M_t));
    page_directory[(vir_addr >> SHIFT_4MB)] = pde_bytes;
    flush_tlb();
}


/*
 * map_user_video_mem
 *   DESCRIPTION: Maps the given virtual address to the video memory and creates
 *                a 4KB page entry for it.
 *   INPUTS: vir_addr - the virtual address within the page to map
 *           pte - the entry to put into the Page Table for this page
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes the x86 TLBs
 */
void
map_user_video_mem(uint32_t vir_addr, pte_t pte)
{
    pte.base_addr = VIDEO_MEM_INDEX;

    uint32_t pte_bytes;
    memcpy(&pte_bytes, &pte, sizeof(pte_t));
    user_4MB_table[(vir_addr >> SHIFT_4KB) & MASK_10_BITS] = pte_bytes;

    pde_4K_t pde;
    memset(&(pde), 0, sizeof(pde_4K_t));
    pde.present = 1;
    pde.read_write = 1;
    pde.user_supervisor = 1;
    pde.base_addr = ((uint32_t) user_4MB_table) >> SHIFT_4KB;
    memcpy(&page_directory[(vir_addr >> SHIFT_4MB)], &pde, sizeof(pde_4K_t));

    flush_tlb();
}


/*
 * free_user_video_mem
 *   DESCRIPTION: Unmaps the page for the 4KB video memory.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes the x86 TLBs
 */
void
free_user_video_mem(uint32_t vir_addr)
{
    pte_t pte;
    memset(&(pte), 0, sizeof(pte_t));
    pte.present = 0;
    pte.read_write = 1;
    pte.user_supervisor = 0;

    uint32_t pte_bytes;
    memcpy(&pte_bytes, &pte, sizeof(pte_t));
    user_4MB_table[(vir_addr >> SHIFT_4KB) & MASK_10_BITS] = pte_bytes;

    flush_tlb();
}


/*
 * map_backup_vidmem TODO
 *   DESCRIPTION: none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes the x86 TLBs
 */
void
map_backup_vidmem(uint32_t vir_addr, uint32_t phys_addr)
{
    pte_t pte;
    memset(&(pte), 0, sizeof(pte_t));

    /* use backup_vidmem_table */
}


/*
 * flush_tlb
 *   DESCRIPTION: Flushes the x86 TLBs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
flush_tlb()
{
    asm volatile(
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %%cr3"
        :
        :
        : "%eax" // clobbers %eax
    );
}
