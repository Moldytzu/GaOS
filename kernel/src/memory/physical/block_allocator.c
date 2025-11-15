#define MODULE "block_allocator"
#include <misc/logger.h>

#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>
#include <arch/arch.h>

#define BLOCK_MINIMUM_ALLOCATION 16
#define BLOCK_SIGNATURE 0x7B3CC1F2D2FACAE0

struct align_addr(16) block_header
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

    // ensure virtual_base is page aligned
    virtual_base = (virtual_base + PAGE - 1) & ~(PAGE - 1);
    block_allocator_current_virtual_address = virtual_base + pages * PAGE;

    // create mappings
    for (size_t p = 0; p < pages; ++p)
        arch_table_manager_map(arch_bootstrap_page_table, virtual_base + p * PAGE, physical_base + p * PAGE, TABLE_ENTRY_READ_WRITE);

    // initialize header area
    block_header_t *hdr = (block_header_t *)virtual_base;
    hdr->signature = BLOCK_SIGNATURE;
    hdr->next = hdr->previous = nullptr;
    hdr->size = pages * PAGE - sizeof(block_header_t);

    // return virtual address
    return (void *)virtual_base;
}

void block_allocator_push_block(block_header_t **list, block_header_t *block)
{
    // insert at head of doubly-linked list
    block->next = *list;
    block->previous = nullptr;
    if (*list)
        (*list)->previous = block;
    *list = block;
}

void block_allocator_remove_block_from_list(block_header_t *block)
{
    // unlink from whatever list it belongs to
    if (block->previous)
        block->previous->next = block->next;
    if (block->next)
        block->next->previous = block->previous;

    if (block == block_free_list_start)
        block_free_list_start = block->next;
    if (block == block_busy_list_start)
        block_busy_list_start = block->next;

    block->next = block->previous = nullptr;
}

block_header_t *block_allocator_create_free_block(size_t pages)
{
    block_header_t *b = (block_header_t *)block_allocator_allocate_block(pages);
    // header already initialized in allocate_block; ensure fields are sane
    b->signature = BLOCK_SIGNATURE;
    b->next = b->previous = nullptr;
    b->size = pages * PAGE - sizeof(block_header_t);

    if (block_free_list_start == nullptr)
    {
        block_free_list_start = b;
        return b;
    }

    block_allocator_push_block(&block_free_list_start, b);
    return b;
}

void block_allocator_move_to_free_list(block_header_t *block)
{
    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_free_list_start, block);
}

void block_allocator_move_to_busy_list(block_header_t *block)
{
    block_allocator_remove_block_from_list(block);
    block_allocator_push_block(&block_busy_list_start, block);
}

void block_allocator_dump_list(block_header_t *list_start)
{
    block_header_t *current_block = list_start;

    while (current_block)
    {
        printk_serial_unsafe("range %p-%p size %zu B (~%zu KB)\n",
                             (void *)((uint64_t)current_block + sizeof(block_header_t)),
                             (void *)((uint64_t)current_block + sizeof(block_header_t) + current_block->size),
                             current_block->size,
                             current_block->size / 1024);
        current_block = current_block->next;
    }
}

void block_allocator_dump_free_list()
{
    printk_serial_unsafe("dumping free list\n");
    block_allocator_dump_list(block_free_list_start);
}

void block_allocator_dump_busy_list()
{
    printk_serial_unsafe("dumping busy list\n");
    block_allocator_dump_list(block_busy_list_start);
}

extern struct limine_memmap_entry **memory_map_entries; // page_allocator.c
extern size_t memory_map_entries_count;
void block_allocator_find_lowest_free_virtual_address_limine()
{
    // we want to find the highest address that's mapped
    // it's guaranteed that the whole memory map is mapped
    // thus we can be sure to choose an address above all of those in the map

    // find highest address in the memory map
    // the protocol guarantees that the entries are sorted by base, lowest to highest
    // thus the highest address should be last entry
    struct limine_memmap_entry *highest_entry = memory_map_entries[memory_map_entries_count - 1];
    block_allocator_virtual_base = block_allocator_current_virtual_address = highest_entry->base + highest_entry->length + kernel_hhdm_offset;

    // align to page
    block_allocator_virtual_base = block_allocator_current_virtual_address = (block_allocator_virtual_base + PAGE - 1) & ~(PAGE - 1);
}

void block_allocator_init()
{
    block_allocator_find_lowest_free_virtual_address_limine();
    block_allocator_create_free_block(1);
    log_info("using virtual base %p", (void *)block_allocator_virtual_base);
}

void *block_allocate(size_t size)
{
    if (size < BLOCK_MINIMUM_ALLOCATION) // clamp the allocation size to the minimum
        size = BLOCK_MINIMUM_ALLOCATION;

    if (size % 16) // round up size to next 16 (0x10)
        size += 16 - size % 16;

    spinlock_acquire(&block_allocator_lock);

    // find first block that fits our size
    block_header_t *current_block = block_free_list_start;
    while (current_block && current_block->size < size)
        current_block = current_block->next;

    if (!current_block) // didn't find a big enough block
    {
        size_t need = size + sizeof(block_header_t);
        size_t pages = (need + PAGE - 1) / PAGE;
        current_block = block_allocator_create_free_block(pages); // create one that fits our needs
    }

    // now current_block holds the block we will allocate

    // try to split if possible
    if (current_block->size > size + sizeof(block_header_t) + BLOCK_MINIMUM_ALLOCATION)
    {
        size_t old_size = current_block->size;                            // hold a copy of the original size
        size_t new_block_size = old_size - size - sizeof(block_header_t); // size of the new block

        current_block->size = size; // clamp the size of the block to the requested size

        block_header_t *new_block = (block_header_t *)((uint64_t)current_block + sizeof(block_header_t) + current_block->size); // move the pointer after the whole block
        new_block->size = new_block_size;                                                                                       // set the size
        new_block->next = current_block->next;
        new_block->previous = current_block;
        new_block->signature = BLOCK_SIGNATURE;
        if (new_block->next)
            new_block->next->previous = new_block;
        current_block->next = new_block;
    }

    block_allocator_move_to_busy_list(current_block); // mark it as busy

    spinlock_release(&block_allocator_lock);

    void *contents = (void *)((uint64_t)current_block + sizeof(block_header_t)); // point after header
    memset(contents, 0, size);                                                   // initialise memory
    return contents;
}

void block_deallocate(void *block)
{
    if (!block)
    {
        log_error("bogus deallocation of nullptr");
        return;
    }

    uint64_t block_virtual_address = (uint64_t)block;
    block_header_t *header = (block_header_t *)((uint64_t)block - sizeof(block_header_t));
    if (header->signature != BLOCK_SIGNATURE)
    {
        log_error("bogus deallocation of %p", (void *)header); // todo: print caller instruction pointer
        return;
    }

    spinlock_acquire(&block_allocator_lock);

    if (block_allocator_virtual_base < block_virtual_address && block_virtual_address < block_allocator_current_virtual_address) // check if the block is in the expected memory region
        block_allocator_move_to_free_list(header);                                                                               // todo: maybe check for merge opportunities?
    else
        log_error("bogus deallocation of %p", (void *)header); // todo: print the caller instruction pointer

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