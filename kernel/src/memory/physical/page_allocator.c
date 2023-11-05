#define MODULE "page_allocator"
#include <misc/logger.h>

#include <misc/panic.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

typedef struct
{
    uint64_t allocate_base;
    uint64_t bitmap_base;
    size_t used;
    size_t available;
    size_t total;
    size_t bitmap_entries;
} page_allocator_pool_t;

struct limine_memmap_entry **memory_map_entries;
size_t memory_map_entries_count;
page_allocator_pool_t allocator_pools[128];
size_t allocator_pool_index = 0;

static void page_allocator_create_pools_limine()
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
        if (required_bitmap_bytes % PAGE) // align to a page's lenght
            required_bitmap_bytes += PAGE - required_bitmap_bytes % PAGE;

        // write the metadata
        pool->bitmap_entries = entry->length / PAGE;
        pool->bitmap_base = entry->base + kernel_hhdm_offset;
        pool->allocate_base = entry->base + required_bitmap_bytes + kernel_hhdm_offset;
        pool->used = 0;
        pool->available = pool->total = entry->length - required_bitmap_bytes;

        // clear the bitmap
        memset((void *)pool->bitmap_base, 0, required_bitmap_bytes);
    }
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

// todo: move these in another source file
static void bitmap_set(uint64_t bitmap, uint64_t index)
{
    uint64_t byte_offset = index / bitsof(uint64_t);
    uint64_t bit = index % bitsof(uint64_t);
    uint64_t mask = 1UL << bit;
    uint64_t *pointer = (uint64_t *)bitmap + byte_offset;

    *pointer |= mask;
}

static void bitmap_unset(uint64_t bitmap, uint64_t index)
{
    uint64_t byte_offset = index / bitsof(uint64_t);
    uint64_t bit = index % bitsof(uint64_t);
    uint64_t mask = 1UL << bit;
    uint64_t *pointer = (uint64_t *)bitmap + byte_offset;

    *pointer &= ~mask;
}

static uint64_t bitmap_get(uint64_t bitmap, uint64_t index)
{
    uint64_t byte_offset = index / bitsof(uint64_t);
    uint64_t bit = index % bitsof(uint64_t);
    uint64_t mask = 1UL << bit;
    uint64_t *pointer = (uint64_t *)bitmap + byte_offset;

    return *pointer & mask;
}

void *page_allocate(size_t pages)
{
    if (!pages)
    {
        log_warn("requested null pages. allocating one page.");
        pages = 1;
    }

    size_t required_bytes = pages * PAGE;

    for (size_t i = 0; i < allocator_pool_index; i++)
    {
        page_allocator_pool_t *pool = &allocator_pools[i];

        if (pool->available < required_bytes) // can't hold the data
            continue;

        // the 'pool' variable holds a valid pool viable for allocation

        // try to find first available page
        for (size_t index = 0; index < pool->bitmap_entries; index++)
        {
            if (bitmap_get(pool->bitmap_base, index))
                continue;

            if (index + pages > pool->bitmap_entries) // doesn't fit
                continue;

            // check if the range is contiguous
            bool can_allocate = true;
            for (size_t offset = 0; offset < pages; offset++)
            {
                if (bitmap_get(pool->bitmap_base, offset + index))
                {
                    can_allocate = false;
                    break;
                }
            }

            if (!can_allocate)
                continue;

            // set the bits
            for (size_t offset = 0; offset < pages; offset++)
                bitmap_set(pool->bitmap_base, offset + index);

            // update metadata
            pool->used += required_bytes;
            pool->available -= required_bytes;

            // return initialised memory
            void *pointer = (void *)(pool->allocate_base + index * PAGE);
            memset(pointer, 0, required_bytes);
            return pointer;
        }
    }

    panic("failed to allocate %d pages (%d KB)", pages, required_bytes / 1024);

    return NULL;
}

void page_deallocate(void *base, size_t pages)
{
    (void)base;
    (void)pages;
}