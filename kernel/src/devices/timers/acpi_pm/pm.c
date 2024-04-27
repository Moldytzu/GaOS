#define MODULE "acpi_pm"
#include <misc/logger.h>

#include <devices/timers/acpi_pm/pm.h>
#include <acpi/acpi.h>
#include <arch/arch.h>

pstruct
{
    acpi_sdt_header_t header;
    uint32_t FACS;
    uint32_t DSDT;
    uint8_t reserved;
    uint8_t prefferedPowerMode;
    uint16_t sciIntrerrupt;
    uint32_t smiCommand;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t s4bios;
    uint8_t pstate;
    uint32_t PM1aEvent;
    uint32_t PM1bEvent;
    uint32_t PM1aControl;
    uint32_t PM1BControl;
    uint32_t PM2Control;
    uint32_t PMTimer;
    uint32_t GPE0;
    uint32_t GPE1;
    uint8_t PM1EventLen;
    uint8_t PM1ControlLen;
    uint8_t PM2ControlLen;
    uint8_t PMTimerLen;
    uint8_t GPE0Len;
    uint8_t GPE1Len;
    uint8_t GPE1Base;
    uint8_t cStateControl;
    uint16_t worstC2;
    uint16_t worstC3;
    uint16_t flushSize;
    uint16_t flushPitch;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
    uint16_t bootFlags;
    uint8_t reserved2;
    uint32_t flags;
    acpi_gas_t reset;
    uint8_t resetValue;
    uint8_t reserved3[3];
    uint64_t FACS64;
    uint64_t DSDT64;
    acpi_gas_t PM1aEvent64;
    acpi_gas_t PM1bEvent64;
    acpi_gas_t PM1aControl64;
    acpi_gas_t PM1BControl64;
    acpi_gas_t PM2Control64;
    acpi_gas_t PMTimer64;
    acpi_gas_t GPE064;
    acpi_gas_t GPE164;
}
acpi_fadt_t;

acpi_fadt_t *acpi_fadt;

uint64_t acpi_pm_timer_read_nanoseconds(void)
{
    uint64_t ticks = 0, nanoseconds = 0;

    if (acpi_revision >= 2)
    {
        switch (acpi_fadt->PMTimer64.address_space)
        {
        case ACPI_ADDRESS_SPACE_SYSTEM_MEMORY:
            ticks = *(uint32_t *)acpi_fadt->PMTimer64.address;
            break;
        case ACPI_ADDRESS_SPACE_I_O:
            ticks = arch_pio_read32(acpi_fadt->PMTimer64.address);
            break;
        }
    }
    else
        ticks = arch_pio_read32(acpi_fadt->PMTimer);

    nanoseconds = ticks * 1000000000ULL / 3579545; // divide by 3.578545 MHz
    return nanoseconds;
}

void acpi_pm_timer_init(void)
{
    acpi_fadt = (acpi_fadt_t *)acpi_get_table("FACP");

    if (!acpi_fadt)
        return;

    if (acpi_fadt->PMTimerLen != 4)
        return;

    // todo: this is unused, find a purpose for this.........
}