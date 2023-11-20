#define MODULE "hpet"
#include <misc/logger.h>

#include <arch/arch.h>
#include <arch/x86_64/timers/hpet.h>
#include <acpi/acpi.h>
#include <clock/clock.h>

#define HPET_OFFSET_GENERAL_CAPABILITIES 0
#define HPET_OFFSET_GENERAL_CONFIGURATION 0x10
#define HPET_OFFSET_GENERAL_INTERRUPT 0x20
#define HPET_OFFSET_MAIN_COUNTER 0xF0
#define HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(n) (0x100 + 0x20 * n)
#define HPET_OFFSET_TIMER_COMPARATOR(n) (0x108 + 0x20 * n)
#define HPET_OFFSET_TIMER_INTERRUPT(n) (0x110 + 0x20 * n)

pstruct
{
    acpi_sdt_header_t header;
    uint32_t event_timer_block_id;
    acpi_gas_t base_address;
    uint8_t hpet_number;
    uint16_t main_counter_minimum_clock_tick_periodic;
    uint8_t page_protection;
}
acpi_hpet_t;

acpi_hpet_t *arch_hpet_header;
uint64_t arch_hpet_ticks_to_nanoseconds;

ifunc void arch_hpet_write(uint64_t offset, uint64_t data)
{
    *((volatile uint64_t *)(arch_hpet_header->base_address.address + offset)) = data;
}

ifunc uint64_t arch_hpet_read(uint64_t offset)
{
    return *((volatile uint64_t *)(arch_hpet_header->base_address.address + offset));
}

uint64_t arch_hpet_read_nanoseconds()
{
    return arch_hpet_read(HPET_OFFSET_MAIN_COUNTER) * arch_hpet_ticks_to_nanoseconds;
}

static clock_time_source_t arch_hpet_timer = {
    .name = "hpet",
    .time_keeping_capable = true,
    .ticks_per_second = 0,
    .read_nanoseconds = arch_hpet_read_nanoseconds,
    .one_shot_capable = false,
};

void arch_hpet_init()
{
    arch_hpet_header = (acpi_hpet_t *)acpi_get_table("HPET");

    if (!arch_hpet_header)
        return;

    if (arch_hpet_header->base_address.address_space != ACPI_ADDRESS_SPACE_SYSTEM_MEMORY) // unsupported address space
        return;

    arch_table_manager_map(arch_bootstrap_page_table, arch_hpet_header->base_address.address, arch_hpet_header->base_address.address, TABLE_ENTRY_READ_WRITE | TABLE_ENTRY_CACHE_DISABLE); // map the base

    uint32_t ticks_to_femtoseconds = arch_hpet_read(HPET_OFFSET_GENERAL_CAPABILITIES) >> 32;
    uint32_t ticks_per_second = 1000000000000000 /*femtosecond to second*/ / ticks_to_femtoseconds; // read the capabilites to calculate the frequency
    arch_hpet_ticks_to_nanoseconds = ticks_to_femtoseconds / 1000000 /*femtosecond to nanosecond*/;

    arch_hpet_timer.ticks_per_second = ticks_per_second; // set the ticks per second in timer structure
    clock_register_time_source(arch_hpet_timer);

    // enable the main counter as periodic without interrupts
    arch_hpet_write(HPET_OFFSET_MAIN_COUNTER, 0);                                        // clear the counter
    arch_hpet_write(HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(0), (1 << 3) | (1 << 6)); // configure the timer (periodic timer-specific configuration)
    arch_hpet_write(HPET_OFFSET_TIMER_COMPARATOR(0), 0);                                 // clear the comparator
    arch_hpet_write(HPET_OFFSET_GENERAL_CONFIGURATION, 0b1);                             // enable counter
}