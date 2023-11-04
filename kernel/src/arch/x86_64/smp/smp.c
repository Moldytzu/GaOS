#define MODULE "smp"
#include <misc/logger.h>

#include <arch/x86_64/arch.h>
#include <acpi/acpi.h>

pstruct
{
    uint8_t type;
    uint8_t length;
}
acpi_madt_entry_t;

pstruct
{
    acpi_sdt_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    acpi_madt_entry_t entries[];
}
acpi_madt_t;

acpi_madt_t *madt;
size_t ap = 0;

int arch_bootstrap_ap()
{
    // discover application processors using madt
    madt = (acpi_madt_t *)acpi_get_table("APIC");

    if (!madt)
        return 0;

    acpi_madt_entry_t *entry = &madt->entries[0];

    while ((uint64_t)entry < (uint64_t)madt + madt->header.length) // loop until we're outside the madt
    {
        if (entry->type == 0) // Processor Local APIC
            ap++;

        entry = (acpi_madt_entry_t *)((uint64_t)entry + entry->length); // point to the next entry
    }

    return ap - 1;
}