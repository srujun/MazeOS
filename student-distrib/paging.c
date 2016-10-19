/*
 * paging.c - Defines function for Paging initialzation
 */

#include "paging.h"

#define PAGE_COUNT         1024
#define PAGE_ALIGN         4096

#define PDE_INIT           0x02  // not present, R/W enabled
#define PTE_VIDEO          0x07  // present, R/W enabled, User access
#define PTE_FIRST_4MB      0x07  // present, R/W enabled, User access
#define KERNEL_MASK        0x83  // PageSize 4MB, R/W enabled, Supervisor access

#define VIDEO_MEM_START    0xB8  // index of start of video memory
#define VIDEO_MEM_PG_COUNT 8     // number of pages in video memory (32KB)

#define KERNEL_MEM_START   0x400000 // start of 4MB Kernel in memory


/* Arrays to hold the Page Directory and the table for the first 4MB section */
uint32_t page_directory[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));
uint32_t first_4MB_table[PAGE_COUNT] __attribute__((aligned(PAGE_ALIGN)));


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

    /* Initialize all pages to be NOT PRESENT,   */
    for (i = 0; i < PAGE_COUNT; i++)
        page_directory[i] = PDE_INIT;

    /* Initialize the 4KB table for the first 4MB in physical memory */
    for (i = 0; i < PAGE_COUNT; i++)
    {
        first_4MB_table[i] = (i * PAGE_ALIGN) | PDE_INIT;
    }

    /* Initialize video memory pages (32KB) starting at 0xB8000 */
    for (i = 0; i < VIDEO_MEM_PG_COUNT; i++)
    {
        first_4MB_table[VIDEO_MEM_START + i] =
            ((VIDEO_MEM_START + i) * PAGE_ALIGN) | PTE_VIDEO;
    }

    /* First Page Directory entry should point to first_4MB_table */
    page_directory[0] = ((uint32_t) first_4MB_table) | PTE_FIRST_4MB;

    /* 4MB for Kernel Space */
    page_directory[1] = KERNEL_MEM_START | KERNEL_MASK;

    /* give the page_directory pointer to CR3 */
    load_page_directory((uint32_t) page_directory);
    /* call the enabler */
    enable_paging();
}
