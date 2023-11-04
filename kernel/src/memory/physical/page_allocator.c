#define MODULE "page_allocator"
#include <misc/logger.h>

#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

struct limine_memmap_entry **memory_map_entries;
size_t memory_map_entries_count;

void page_allocator_create_pools_limine()
{
    memory_map_entries = kernel_memmap_request.response->entries;
    memory_map_entries_count = kernel_memmap_request.response->entry_count;

    log_info("%d memory map entries", memory_map_entries_count);
}

void page_allocator_init()
{
    page_allocator_create_pools_limine();
}

void *page_allocate(size_t pages)
{
    (void)pages;
    return NULL;
}

void page_deallocate(void *base, size_t pages)
{
    (void)base;
    (void)pages;
}