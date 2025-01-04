#define MODULE "x86_64/table_manager"
#include <misc/logger.h>

#include <arch/x86_64/page_table_manager/table_manager.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

arch_page_table_t *arch_bootstrap_page_table;

ifunc void arch_table_manager_set_address(uint64_t *entry, uint64_t address)
{
    // note: address here is a page number, max physical address will be 40 + 12 = 52 bits (4096 TB!)
    *entry &= 0xFFF0000000000FFF; // clear old address field
    *entry |= address;            // put the new address
}

ifunc arch_page_table_layer_t *arch_table_manager_allocate_next_layer(arch_page_table_layer_t *layer, uint64_t index, uint64_t flags)
{
    uint64_t *entry = &layer->entries[index]; // index next layer's entry

    flags &= 0xFF; // drop the nx bit

    // allocate it if needed
    if (!(*entry & TABLE_ENTRY_PRESENT))
    {
        uint64_t newLayer = (uint64_t)page_allocate(1); // allocate a new layer
        newLayer -= kernel_hhdm_offset;                 // get physical address of it
        *entry |= newLayer;                             // set the page address
        *entry |= flags | TABLE_ENTRY_PRESENT;          // set flags
    }
    else
        *entry |= flags;

    return (arch_page_table_layer_t *)(arch_table_manager_get_address(entry) + kernel_hhdm_offset); // return its address
}

void arch_table_manager_map(arch_page_table_t *table, uint64_t virtual_address, uint64_t physical_address, uint64_t flags)
{
    // generate indices
    // uint64_t pml4 = (virtual_address & (0x1FFULL << 48)) >> 48; // for future use (5 level paging)
    uint64_t pml3_index = (virtual_address & (0x1FFULL << 39)) >> 39;
    uint64_t pml2_index = (virtual_address & (0x1FFULL << 30)) >> 30;
    uint64_t pml1_index = (virtual_address & (0x1FFULL << 21)) >> 21;
    uint64_t p_index = (virtual_address & (0x1FFULL << 12)) >> 12;

    // traverse the page table while allocating if needed
    arch_page_table_layer_t *pml3 = arch_table_manager_allocate_next_layer(table, pml3_index, flags);
    arch_page_table_layer_t *pml2 = arch_table_manager_allocate_next_layer(pml3, pml2_index, flags);
    arch_page_table_layer_t *pml1 = arch_table_manager_allocate_next_layer(pml2, pml1_index, flags);

    uint64_t *entry = &pml1->entries[p_index];
    *entry |= flags | TABLE_ENTRY_PRESENT;
    arch_table_manager_set_address(entry, physical_address);
}

uint64_t arch_table_manager_translate_to_physical(arch_page_table_t *table, uint64_t virtual_address)
{
    // generate indices
    // uint64_t pml4 = (virtual_address & (0x1FFULL << 48)) >> 48; // for future use (5 level paging)
    uint64_t pml3_index = (virtual_address & (0x1FFULL << 39)) >> 39;
    uint64_t pml2_index = (virtual_address & (0x1FFULL << 30)) >> 30;
    uint64_t pml1_index = (virtual_address & (0x1FFULL << 21)) >> 21;
    uint64_t p_index = (virtual_address & (0x1FFULL << 12)) >> 12;

    // traverse the page table while checking if the layer is allocated
    arch_page_table_layer_t *pml3 = arch_table_manager_get_next_layer(table, pml3_index);
    if (pml3 == nullptr)
        return 0;

    arch_page_table_layer_t *pml2 = arch_table_manager_get_next_layer(pml3, pml2_index);
    if (pml2 == nullptr)
        return 0;

    arch_page_table_layer_t *pml1 = arch_table_manager_get_next_layer(pml2, pml1_index);
    if (pml1 == nullptr)
        return 0;

    uint64_t *entry = &pml1->entries[p_index];
    if (*entry & TABLE_ENTRY_PRESENT)
        return arch_table_manager_get_address(entry);
    else
        return 0;
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