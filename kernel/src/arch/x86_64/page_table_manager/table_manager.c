#define MODULE "x86_64/table_manager"
#include <misc/logger.h>

#include <arch/x86_64/page_table_manager/table_manager.h>
#include <memory/physical/page_allocator.h>

#define TABLE_ENTRY_PRESENT (1 << 0)
#define TABLE_ENTRY_READ_WRITE (1 << 1)
#define TABLE_ENTRY_READ_ONLY 0
#define TABLE_ENTRY_USER (1 << 2)
#define TABLE_ENTRY_WRITE_THROUGH (1 << 3)
#define TABLE_ENTRY_CACHE_DISABLE (1 << 4)
#define TABLE_ENTRY_ACCESSED (1 << 5)
#define TABLE_ENTRY_DIRTY (1 << 6)
#define TABLE_ENTRY_HUGE_PAGES (1 << 7)
#define TABLE_ENTRY_NO_EXECUTE (1 << 63)

pstruct
{
    uint64_t entries[512];
}
arch_page_table_t;

static void arch_table_manager_set_address(uint64_t *entry, uint64_t address)
{
    address &= 0xFFFFFFFFFF;      // clamp the address to 40-bit physical
    *entry &= 0xFFF0000000000FFF; // clear old address field
    *entry |= address << 12;      // put the new address
}

static uint64_t arch_table_manager_get_address(uint64_t *entry)
{
    return (*entry >> 12) & 0xFFFFFFFFFF;
}

void arch_table_manager_init()
{
}