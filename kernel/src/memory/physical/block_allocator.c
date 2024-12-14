#define MODULE "block_allocator"
#include <misc/logger.h>

#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>
#include <arch/arch.h>

#define BLOCK_MINIMUM_ALLOCATION 16
#define BLOCK_SIGNATURE 0x7B3CC1F2D2FACAE0

struct block_header
{
    uint64_t signature;
    size_t size;
    struct block_header *next;
    struct block_header *previous align_addr(16); // ensure alignment
};

typedef struct block_header block_header_t; // create a type for the block header structure

block_header_t *block_free_list_start = nullptr;
block_header_t *block_busy_list_start = nullptr;

uint64_t block_allocator_virtual_base = 0;
uint64_t block_allocator_current_virtual_address = 0;

spinlock_t block_allocator_lock;

void *block_allocator_allocate_block(size_t pages)
{
    uint64_t physical_base = (uint64_t)page_allocate(pages) - kernel_hhdm_offset; // allocate physical memory
    uint64_t virtual_base = block_allocator_current_virtual_address;              // allocate a virtual address
    block_allocator_current_virtual_address += pages * PAGE;

    // create mappings
    for (size_t i = 0; i < pages * PAGE; i += PAGE)
        arch_table_manager_map(arch_bootstrap_page_table, virtual_base + i, physical_base + i, TABLE_ENTRY_READ_WRITE);

    // return virtual address
    return (void *)virtual_base;
}

void block_allocator_push_block(block_header_t **list, block_header_t *block)
{
    block->next = block->previous = nullptr; // make sure the links aren't present

    if (*list) // if the list exists
    {
        // point to the last block
        block_header_t *current = *list;
        while (current->next)
            current = current->next;

        // add the block to the last block
        block->previous = current;
        current->next = block;
    }
    else
        *list = block; // make the list the block itself
}

void block_allocator_remove_block_from_list(block_header_t *block)
{
    // handles all the cases needed when removing a block from a list

    if (block == block_free_list_start)
        block_free_list_start = block->next;
    else if (block == block_busy_list_start)
        block_busy_list_start = block->next;
    else
    {
        if (block->previous)
            block->previous->next = block->next;
        if (block->next)
            block->next->previous = block->previous;
    }
}

block_header_t *block_allocator_create_free_block(size_t pages)
{
    if (block_free_list_start == nullptr)
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
        new_block->signature = BLOCK_SIGNATURE;

        block_allocator_push_block(&block_free_list_start, new_block); // push it on the list

        return new_block;
    }
}

void block_allocator_move_to_free_list(block_header_t *block)
{
    if (block->previous == nullptr && block->next == nullptr)
        block_busy_list_start = nullptr;

    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_free_list_start, block);
}

void block_allocator_move_to_busy_list(block_header_t *block)
{
    if (block->previous == nullptr && block->next == nullptr)
        block_free_list_start = nullptr;

    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_busy_list_start, block);
}

void block_allocator_dump_list(block_header_t *list_start)
{
    block_header_t *current_block = list_start;

    while (current_block)
    {
        printk_serial("range %p-%p size %d B (~%d KB)\n", (uint64_t)current_block + sizeof(block_header_t), (uint64_t)current_block + sizeof(block_header_t) + current_block->size, current_block->size, current_block->size / 1024);
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
    // we want to find the highest address that's mapped
    // it's guranteed that the whole memory map is mapped
    // thus we can be sure to choose an address above all of those in the map

    // find highest address in the memory map
    // the protocol gurantees that the entries are sorted by base, lowest to highest
    // thus the highest address should be last entry
    struct limine_memmap_entry *highest_entry = memory_map_entries[memory_map_entries_count - 1];
    block_allocator_virtual_base = block_allocator_current_virtual_address = highest_entry->base + highest_entry->length + kernel_hhdm_offset;
}

void block_allocator_init()
{
    block_allocator_find_lowest_free_virtual_address_limine();
    log_info("using virtual base %p", block_allocator_virtual_base);
}

void *block_allocate(size_t size)
{
    if (size < BLOCK_MINIMUM_ALLOCATION) // clamp the allocation size to the minimum
        size = BLOCK_MINIMUM_ALLOCATION;

    if (size % 16) // round up size to next 16 (0x10)
        size += 16 - size % 16;

    spinlock_acquire(&block_allocator_lock);

    if (block_free_list_start == nullptr)                   // no blocks available
        block_allocator_create_free_block(size / PAGE + 1); // create one that fits our needs

    // find first block that fits our size
    block_header_t *current_block = block_free_list_start;
    while (current_block && current_block->size < size)
        current_block = current_block->next;

    if (!current_block)                                                     // didn't find a big enough block
        current_block = block_allocator_create_free_block(size / PAGE + 1); // create one that fits our needs

    // now current_block holds the block we will allocate

    // try to split if possible
    if (current_block->size > size + sizeof(block_header_t) + BLOCK_MINIMUM_ALLOCATION)
    {
        size_t old_size = current_block->size;                            // hold a copy of the original size
        size_t new_block_size = old_size - size - sizeof(block_header_t); // size of the new block

        current_block->size = size; // clamp the size of the block to the requested size

        block_header_t *new_block = (block_header_t *)((uint64_t)current_block + sizeof(block_header_t) + current_block->size); // move the pointer after the whole block
        new_block->size = new_block_size;                                                                                       // set the size
        new_block->next = current_block->next;                                                                                  // link it in the list
        new_block->previous = current_block;                                                                                    //
        new_block->signature = BLOCK_SIGNATURE;                                                                                 // set the signature
        current_block->next = new_block;                                                                                        // make the block refer to the newly created block
    }

    block_allocator_move_to_busy_list(current_block); // mark it as busy

    spinlock_release(&block_allocator_lock);

    void *contents = (void *)((uint64_t)current_block + sizeof(block_header_t)); // point after header
    memset(contents, 0, size);                                                   // initialise memory
    return contents;
}

void block_deallocate(void *block)
{
    uint64_t block_virtual_address = (uint64_t)block;
    block_header_t *header = (block_header_t *)((uint64_t)block - sizeof(block_header_t));
    if (header->signature != BLOCK_SIGNATURE)
    {
        log_error("bogus deallocation of %p", header); // todo: print caller instruction pointer
        return;
    }

    spinlock_acquire(&block_allocator_lock);

    if (block_allocator_virtual_base < block_virtual_address && block_virtual_address < block_allocator_current_virtual_address) // check if the block is in the expected memory region
        block_allocator_move_to_free_list(header);                                                                               // todo: maybe check for merge oportunities?
    else
        log_error("bogus deallocation of %p", header); // todo: print the caller instruction pointer

    spinlock_release(&block_allocator_lock);
}

void block_allocate_dump_usage()
{
    spinlock_acquire(&block_allocator_lock);
    size_t used = 0, used_blocks = 0;
    size_t free = 0, free_blocks = 0;

    block_header_t *list = block_free_list_start;
    while (list)
    {
        free += list->size;
        free_blocks++;
        list = list->next;
    }

    list = block_busy_list_start;
    while (list)
    {
        used += list->size;
        used_blocks++;
        list = list->next;
    }

    log_info("%db used (%d blocks) %db free (%d blocks)", used, used_blocks, free, free_blocks);
    spinlock_release(&block_allocator_lock);
}