#define MODULE "page_allocator"
#include <misc/logger.h>

#include <misc/panic.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>
#include <arch/arch.h>

typedef struct
{
    uint64_t allocate_base;
    uint64_t bitmap_base;
    size_t used;
    size_t available;
    size_t total;
    size_t bitmap_entries;

    // for optimizations
    size_t last_allocated_page_index;
} page_allocator_pool_t;

struct limine_memmap_entry **memory_map_entries;
size_t memory_map_entries_count;
page_allocator_pool_t allocator_pools[128];
size_t allocator_pool_index = 0;
arch_spinlock_t page_allocator_lock;

static void page_allocator_create_pools_limine(void)
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
        pool->last_allocated_page_index = 0;

        // clear the bitmap
        zero64((void *)pool->bitmap_base, required_bitmap_bytes);
    }
}

void page_allocator_init(void)
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

    arch_spinlock_acquire(&page_allocator_lock);

    size_t required_bytes = pages * PAGE;

    for (size_t i = 0; i < allocator_pool_index; i++)
    {
        page_allocator_pool_t *pool = &allocator_pools[i];

        if (pool->available < required_bytes) // can't hold the data
            continue;

        // the 'pool' variable holds a valid pool viable for allocation

        // try to find first available page
        for (size_t index = pool->last_allocated_page_index; index < pool->bitmap_entries; index++)
        {
            if (bitmap_get(pool->bitmap_base, index))
                continue;

            if (index + pages > pool->bitmap_entries) // doesn't fit
                continue;

            // check if the range is contiguous
            for (size_t offset = 0; offset < pages; offset++)
                if (bitmap_get(pool->bitmap_base, offset + index)) // this page is allocated
                    goto try_next;                                 // try next offset

            // set the bits
            for (size_t offset = 0; offset < pages; offset++)
                bitmap_set(pool->bitmap_base, offset + index);

            // update metadata
            pool->used += required_bytes;
            pool->available -= required_bytes;
            pool->last_allocated_page_index = index;

            // return initialised memory
            void *pointer = (void *)(pool->allocate_base + index * PAGE);
            zero64(pointer, required_bytes);

            // printk_serial("page_allocator: allocating %p (%d pages)\n", pointer, pages);

            arch_spinlock_release(&page_allocator_lock);

            return pointer;

        try_next:
        }
    }

    arch_spinlock_release(&page_allocator_lock);

    panic("failed to allocate %d pages (%d KB)", pages, required_bytes / 1024);

    return NULL;
}

void page_deallocate(void *base, size_t pages)
{
    arch_spinlock_acquire(&page_allocator_lock);

    uint64_t base_int = (uint64_t)base;
    for (size_t i = 0; i < allocator_pool_index; i++)
    {
        page_allocator_pool_t *pool = &allocator_pools[i];

        if (pool->allocate_base <= base_int && base_int <= pool->allocate_base + pool->total)
        {
            // found the correct pool

            size_t index = (base_int - pool->allocate_base) / PAGE; // use the reverse equation to determine the page index

            for (size_t offset = 0; offset < pages; offset++)
            {
                if (!bitmap_get(pool->bitmap_base, index + offset)) // oops... this page was already deallocated
                {
                    log_error("failed to deallocate already deallocated page at %p", base); // todo: print the caller instruction pointer
                    continue;
                }

                bitmap_unset(pool->bitmap_base, index + offset);
            }

            arch_spinlock_release(&page_allocator_lock);

            return;
        }
    }

    log_error("bogus deallocation of %p", base); // todo: print the caller instruction pointer

    arch_spinlock_release(&page_allocator_lock);
}