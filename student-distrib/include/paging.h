/*
 * paging.h - Declares functions for Paging initialzation
 */

#ifndef PAGING_H
#define PAGING_H

#include "types.h"

typedef struct __attribute__((packed)) pde {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t writethrough : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t attr_index : 1;
    uint32_t reserved : 9;
    uint32_t base_addr : 10;
} pde_t;

typedef struct __attribute__((packed)) pte {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t writethrough : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t base_addr : 20;
} pte_t;

/* Externally visible functions */

void init_paging(void);

void map_pde(uint32_t vir_addr, pde_t pde);
void map_user_video_mem(uint32_t vir_addr, pte_t pte);
void flush_tlb();

/* Functions defined in Assembly */
extern void load_page_directory(uint32_t pagedir_addr);
extern void enable_paging(void);

#endif
