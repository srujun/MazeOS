/*
 * paging.h - Declares functions for Paging initialzation
 */

#ifndef PAGING_H
#define PAGING_H

#include "types.h"

/* Externally visible functions */

void init_paging(void);

/* Functions defined in Assembly */
extern void load_page_directory(uint32_t pagedir_addr);
extern void enable_paging(void);

#endif
