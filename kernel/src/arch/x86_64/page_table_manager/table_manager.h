#pragma once
#include <misc/libc.h>
#include <boot/limine.h>

#ifndef PAGE
#include <arch/arch.h>
#endif

#define TABLE_ENTRY_PRESENT (1 << 0)
#define TABLE_ENTRY_READ_WRITE (1 << 1)
#define TABLE_ENTRY_READ_ONLY 0
#define TABLE_ENTRY_USER (1 << 2)
#define TABLE_ENTRY_WRITE_THROUGH (1 << 3)
#define TABLE_ENTRY_CACHE_DISABLE (1 << 4)
#define TABLE_ENTRY_ACCESSED (1 << 5)
#define TABLE_ENTRY_DIRTY (1 << 6)
#define TABLE_ENTRY_HUGE_PAGES (1 << 7)
#define TABLE_ENTRY_NO_EXECUTE (1ULL << 63)

typedef struct
{
    uint64_t entries[512];
} arch_page_table_t;

typedef arch_page_table_t arch_page_table_layer_t;
extern arch_page_table_t *arch_bootstrap_page_table;

ifunc void arch_table_manager_switch_to(arch_page_table_t *table)
{
    iasm("mov %0, %%cr3" ::"r"((uint64_t)table - kernel_hhdm_offset) : "memory");
}

void arch_table_manager_map(arch_page_table_t *table, uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
void arch_table_manager_init(void);
uint64_t arch_table_manager_translate_to_physical(arch_page_table_t *table, uint64_t virtual_address);
arch_page_table_t *arch_table_manager_new(void);