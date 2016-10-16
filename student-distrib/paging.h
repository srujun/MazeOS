#ifndef PAGING_H
#define PAGING_H

#include "types.h"

void init_paging(void);

extern void load_page_directory(uint32_t pagedir_addr);

extern void enable_paging(void);

#endif
