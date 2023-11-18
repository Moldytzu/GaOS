#define MODULE "block_allocator"
#include <misc/logger.h>

#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>
#include <arch/arch.h>

struct block_header
{
    size_t size;
    struct block_header *next;
    struct block_header *previous align_addr(16); // ensure alignment
};

typedef struct block_header block_header_t; // create a type for the block header structure

// fixme: all of these functions aren't thread-safe!
block_header_t *block_free_list_start = NULL;
block_header_t *block_busy_list_start = NULL;

uint64_t block_allocator_virtual_base = 0;

void *block_allocator_allocate_block(size_t pages)
{
    uint64_t physical_base = (uint64_t)page_allocate(pages) - kernel_hhdm_offset;
    uint64_t virtual_base = block_allocator_virtual_base;
    block_allocator_virtual_base += pages * PAGE;

    for (size_t i = 0; i < pages * PAGE; i += PAGE)
        arch_table_manager_map(arch_bootstrap_page_table, virtual_base + i, physical_base + i, TABLE_ENTRY_READ_WRITE);

    return (void *)virtual_base;
}

void block_allocator_push_block(block_header_t **list, block_header_t *block)
{
    if (*list != NULL)
    {
        // point to the last block in list
        block_header_t *current_block = *list;

        while (current_block->next)
            current_block = current_block->next;

        // push the block
        current_block->next = block;
        block->previous = current_block;
        block->next = NULL;
    }
    else
    {
        block->next = block->previous = NULL;
        *list = block;
    }
}

void block_allocator_remove_block_from_list(block_header_t *block)
{
    if (block->previous)
        block->previous->next = block->next;
    if (block->next)
        block->next->previous = block->previous;
}

block_header_t *block_allocator_create_free_block(size_t pages)
{
    if (block_free_list_start == NULL)
    {
        // there isn't a start
        // we have to allocate one
        block_free_list_start = block_allocator_allocate_block(pages);
        block_free_list_start->size = pages * PAGE - sizeof(block_header_t);

        return block_free_list_start;
    }
    else
    {
        // create a new block
        block_header_t *new_block = block_allocator_allocate_block(pages);
        new_block->size = pages * PAGE - sizeof(block_header_t);

        block_allocator_push_block(&block_free_list_start, new_block); // push it on the list

        return new_block;
    }
}

void block_allocator_move_to_free_list(block_header_t *block)
{
    if (block->previous == NULL && block->next == NULL)
        block_busy_list_start = NULL;

    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_free_list_start, block);
}

void block_allocator_move_to_busy_list(block_header_t *block)
{
    if (block->previous == NULL && block->next == NULL)
        block_free_list_start = NULL;

    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_busy_list_start, block);
}

void block_allocator_dump_list(block_header_t *list_start)
{
    block_header_t *current_block = list_start;

    while (current_block)
    {
        printk_serial("range 0x%p-0x%p size %d B (~%d KB)\n", (uint64_t)current_block + sizeof(block_header_t), (uint64_t)current_block + sizeof(block_header_t) + current_block->size, current_block->size, current_block->size / 1024);
        current_block = current_block->next;
    }
}

void block_allocator_dump_free_list()
{
    printk_serial("dumping free list\n");
    block_allocator_dump_list(block_free_list_start);
}

void block_allocator_dump_busy_list()
{
    printk_serial("dumping busy list\n");
    block_allocator_dump_list(block_busy_list_start);
}

extern struct limine_memmap_entry **memory_map_entries; // page_allocator.c
extern size_t memory_map_entries_count;
void block_allocator_find_lowest_free_virtual_address_limine()
{
    // find highest address in the memory map
    // the protocol gurantees that the entries are sorted by base, lowest to highest
    // thus the highest address should be last entry
    struct limine_memmap_entry *highest_entry = memory_map_entries[memory_map_entries_count - 1];
    block_allocator_virtual_base = highest_entry->base + highest_entry->length + kernel_hhdm_offset;
}

void block_allocator_init()
{
    block_allocator_find_lowest_free_virtual_address_limine();
    log_info("using virtual base 0x%p", block_allocator_virtual_base);

    block_allocator_create_free_block(1);
    block_allocator_dump_free_list();
}

void *block_allocate(size_t size)
{
    (void)size;
    return NULL;
}

void block_deallocate(void *block)
{
    (void)block;
}