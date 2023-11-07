#define MODULE "x86_64/xapic"
#include <misc/logger.h>
#include <misc/panic.h>

#include <arch/x86_64/page_table_manager/table_manager.h>
#include <arch/x86_64/xapic/xapic.h>
#include <arch/x86_64/pio.h>
#include <arch/x86_64/msr.h>

uint64_t arch_get_id()
{
    return xapic_read(XAPIC_REG_ID) >> 24;
}

void arch_xapic_init(bool bsp)
{
    if ((rdmsr(MSR_APIC_BASE) & 0xFFFFF000) != (uint64_t)XAPIC_BASE)
        panic("Out of spec xapic address. 0x%p != 0x%p", rdmsr(MSR_APIC_BASE) & 0xFFFFF000, (uint64_t)XAPIC_BASE);

    if (bsp)
    {
        // mask the PIC if any
        arch_pio_write8(0x20, 0b11111111);
        arch_pio_write8(0xA0, 0b11111111);

        arch_table_manager_map(arch_bootstrap_page_table, XAPIC_BASE, XAPIC_BASE, TABLE_ENTRY_READ_WRITE | TABLE_ENTRY_CACHE_DISABLE); // map the base
    }

    // reset important registers to a known state before enabling the apic (not required by any spec)
    xapic_write(XAPIC_REG_DFR, 0xFF000000);
    xapic_write(XAPIC_REG_LDR, 0x01000000);
    xapic_write(XAPIC_REG_LVT0, 0x00010000);
    xapic_write(XAPIC_REG_LVT1, 0x00010000);
    xapic_write(XAPIC_REG_TPR, 0);

    // enable the LAPIC in XAPIC mode (11.4.3 Volume 3 Intel SDM)
    uint64_t base = rdmsr(MSR_APIC_BASE) | 0b100000000000; // set global enable flag

    if (bsp) // set the bsp flag
        base |= (uint32_t)0b100000000;

    wrmsr(MSR_APIC_BASE, base); // write back the base

    xapic_write(XAPIC_REG_SIV, 0x120); // software enable apic and set the spurious vector to 0x20

    log_info("enabled for %x", arch_get_id());
}