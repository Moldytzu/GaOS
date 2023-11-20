#define MODULE "hpet"
#include <misc/logger.h>

#include <arch/x86_64/timers/hpet.h>
#include <acpi/acpi.h>

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

void arch_hpet_init()
{
    arch_hpet_header = (acpi_hpet_t *)acpi_get_table("HPET");

    if (!arch_hpet_header)
        return;
}