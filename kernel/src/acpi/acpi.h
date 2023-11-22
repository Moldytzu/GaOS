#pragma once
#include <misc/libc.h>

#define ACPI_ADDRESS_SPACE_SYSTEM_MEMORY 0
#define ACPI_ADDRESS_SPACE_I_O 1

pstruct
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem[6];
    char oem_table[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
}
acpi_sdt_header_t;

pstruct
{
    uint8_t address_space;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
}
acpi_gas_t;

extern uint8_t acpi_revision;
void acpi_init();
acpi_sdt_header_t *acpi_get_table(char *signature);