#define MODULE "page_allocator"
#include <misc/logger.h>

#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

typedef struct
{
    uint64_t allocate_base;
    uint64_t bitmap_base;
    size_t used;
    size_t available;
    size_t total;
    bool busy;
} page_allocator_pool_t;

struct limine_memmap_entry **memory_map_entries;
size_t memory_map_entries_count;
page_allocator_pool_t allocator_pools[128];
size_t allocator_pool_index = 0;

void page_allocator_create_pools_limine()
{
    memory_map_entries = kernel_memmap_request.response->entries;
    memory_map_entries_count = kernel_memmap_request.response->entry_count;

    log_info("%d memory map entries", memory_map_entries_count);

    // parse the memory map
    for (size_t i = 0; i < memory_map_entries_count; i++)
    {
        struct limine_memmap_entry *entry = memory_map_entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;

        page_allocator_pool_t *pool = &allocator_pools[allocator_pool_index++];

        // determine the required bytes for the bitmap
        size_t required_bitmap_bytes = entry->length / PAGE / bitsof(uint8_t);
        if (required_bitmap_bytes % 4096)
            required_bitmap_bytes += 4096 - required_bitmap_bytes % 4096;

        // write the metadata
        pool->bitmap_base = entry->base + kernel_hhdm_offset;
        pool->allocate_base = entry->base + required_bitmap_bytes;
        pool->used = 0;
        pool->available = pool->total = entry->length - required_bitmap_bytes;
        pool->busy = false;

        // clear the bitmap
        memset((void *)pool->bitmap_base, 0, required_bitmap_bytes);
    }

    log_info("%d KB to be reclaimed", to_reclaim / 1024);
}

void page_allocator_init()
{
    page_allocator_create_pools_limine();

    // display all pools and calculate the total memory available
    size_t total_ram = 0;
    for (size_t i = 0; i < allocator_pool_index; i++)
    {
        page_allocator_pool_t *pool = &allocator_pools[i];
        log_info("pool %d: %p-%p (%d KB)", i, pool->allocate_base, pool->allocate_base + pool->available, pool->available / 1024);

        total_ram += pool->total;
    }

    log_info("%d MB system", total_ram / 1024 / 1024);
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