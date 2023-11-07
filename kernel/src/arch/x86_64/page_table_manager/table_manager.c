#define MODULE "x86_64/table_manager"
#include <misc/logger.h>

#include <arch/x86_64/page_table_manager/table_manager.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

typedef struct
{
    uint64_t pml4;
    uint64_t pml3;
    uint64_t pml2;
    uint64_t pml1;
    uint64_t p;
} arch_table_indexer_t;

arch_page_table_t *arch_bootstrap_page_table;

static void arch_table_manager_index(arch_table_indexer_t *index, uint64_t virtual_address)
{
    index->p = (virtual_address & (0x1FFULL << 12)) >> 12;
    index->pml1 = (virtual_address & (0x1FFULL << 21)) >> 21;
    index->pml2 = (virtual_address & (0x1FFULL << 30)) >> 30;
    index->pml3 = (virtual_address & (0x1FFULL << 39)) >> 39;
    index->pml4 = (virtual_address & (0x1FFULL << 48)) >> 48;
}

static void arch_table_manager_set_address(uint64_t *entry, uint64_t address)
{
    address &= 0xFFFFFFFFFF;      // clamp the address to 40-bit physical
    *entry &= 0xFFF0000000000FFF; // clear old address field
    *entry |= address << 12;      // put the new address
}

static uint64_t arch_table_manager_get_address(uint64_t *entry)
{
    return ((*entry >> 12) & 0xFFFFFFFFFF) << 12;
}

static void arch_table_manager_switch_to(arch_page_table_t *table)
{
    iasm("mov %0, %%cr3" ::"r"((uint64_t)table - kernel_hhdm_offset) : "memory");
}

arch_page_table_layer_t *arch_table_manager_next_layer(arch_page_table_layer_t *layer, uint64_t index, uint64_t flags)
{
    uint64_t *entry = &layer->entries[index]; // index next layer's entry

    // allocate it if needed
    if (!(*entry & TABLE_ENTRY_PRESENT))
    {
        uint64_t newLayer = (uint64_t)page_allocate(1);        // allocate a new layer
        newLayer -= kernel_hhdm_offset;                        // get physical address of it
        arch_table_manager_set_address(entry, newLayer >> 12); // set the page address
        *entry |= TABLE_ENTRY_PRESENT;                         // set present bit
    }

    *entry |= flags;

    return (arch_page_table_layer_t *)(arch_table_manager_get_address(entry) + kernel_hhdm_offset); // return its address
}

void arch_table_manager_map(arch_page_table_t *table, uint64_t virtual_address, uint64_t physical_address, uint64_t flags)
{
    // generate indices
    arch_table_indexer_t indexer;
    arch_table_manager_index(&indexer, virtual_address);

    // traverse the page table
    arch_page_table_layer_t *pml3 = arch_table_manager_next_layer(table, indexer.pml3, flags);
    arch_page_table_layer_t *pml2 = arch_table_manager_next_layer(pml3, indexer.pml2, flags);
    arch_page_table_layer_t *pml1 = arch_table_manager_next_layer(pml2, indexer.pml1, flags);

    uint64_t *entry = &pml1->entries[indexer.p];
    *entry |= flags | TABLE_ENTRY_PRESENT;
    arch_table_manager_set_address(entry, physical_address >> 12);
}

arch_page_table_t *arch_table_manager_new()
{
    return page_allocate(1);
}

void arch_table_manager_create_bootstrap_table_limine()
{
    arch_bootstrap_page_table = arch_table_manager_new();

    // map the memory map in hhdm
    struct limine_memmap_entry **memory_map_entries = kernel_memmap_request.response->entries;
    size_t memory_map_entries_count = kernel_memmap_request.response->entry_count;
    uint64_t kernel_physical_base = kernel_kernel_address_request.response->physical_base;
    uint64_t kernel_virtual_base = kernel_kernel_address_request.response->virtual_base;

    for (size_t i = 0; i < memory_map_entries_count; i++)
    {
        struct limine_memmap_entry *entry = memory_map_entries[i];

        // round down the base
        uint64_t physical_base = entry->base;
        if (physical_base % PAGE)
            physical_base -= physical_base % PAGE;

        uint64_t virtual_base = physical_base + kernel_hhdm_offset;

        if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
        {
            for (size_t offset = 0; offset < entry->length; offset += PAGE)
                arch_table_manager_map(arch_bootstrap_page_table, kernel_virtual_base + offset, kernel_physical_base + offset, TABLE_ENTRY_READ_WRITE);
        }
        else
        {
            for (size_t offset = 0; offset < entry->length; offset += PAGE)
                arch_table_manager_map(arch_bootstrap_page_table, virtual_base + offset, physical_base + offset, TABLE_ENTRY_READ_WRITE);
        }
    }
}

void arch_table_manager_init()
{
    arch_table_manager_create_bootstrap_table_limine();
    arch_table_manager_switch_to(arch_bootstrap_page_table);
}