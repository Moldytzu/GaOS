#define MODULE "x86_64/smp"
#include <misc/logger.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/xapic/xapic.h>
#include <acpi/acpi.h>
#include <misc/libc.h>
#include <boot/limine.h>

pstruct
{
    uint8_t type;
    uint8_t length;
}
acpi_madt_entry_t;

pstruct
{
    acpi_madt_entry_t header;
    uint8_t acpi_id;
    uint8_t apic_id;
    uint32_t flags;
}
acpi_madt_processor_entry_t;

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

extern void arch_ap_entry16();

int arch_bootstrap_ap()
{
    // copy trampoline
    memcpy((void *)(arch_trampoline_base + kernel_hhdm_offset), arch_ap_entry16, PAGE);

    // discover application processors using madt
    madt = (acpi_madt_t *)acpi_get_table("APIC");

    if (!madt)
        return 0;

    acpi_madt_entry_t *entry = &madt->entries[0];

    while ((uint64_t)entry < (uint64_t)madt + madt->header.length) // loop until we're outside the madt
    {
        if (entry->type == 0) // Processor Local APIC
        {
            acpi_madt_processor_entry_t *processor = (acpi_madt_processor_entry_t *)entry;

            // check the flags to see if the processor can be enabled
            bool enabled = processor->flags & 1;
            bool online_capable = processor->flags & 0b10;
            uint64_t id = processor->apic_id;

            // enable if it's not bsp and possible
            if ((enabled || online_capable) && (id != arch_get_id()))
            {
                arch_hint_serialize();

                xapic_write(XAPIC_REG_ICR_HIGH, id << 24);
                xapic_write(XAPIC_REG_ICR_LOW, 0b11 << 14 /*assert level*/ | 0b101 /*INIT*/ << 8);

                arch_hint_serialize();

                for (int i = 0; i < 100000; i++)
                    arch_hint_spinlock();

                xapic_write(XAPIC_REG_ICR_HIGH, id << 24);
                xapic_write(XAPIC_REG_ICR_LOW, arch_trampoline_base >> 12 | 0b11 << 14 /*assert level*/ | 0b110 /*Start-Up*/ << 8);

                for (int i = 0; i < 100000; i++)
                    arch_hint_spinlock();

                arch_hint_serialize();

                ap++;
            }
        }

        entry = (acpi_madt_entry_t *)((uint64_t)entry + entry->length); // point to the next entry
    }

    return ap - 1;
}