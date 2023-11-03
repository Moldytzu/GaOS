#pragma once
#include <misc/libc.h>

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

void acpi_init();
acpi_sdt_header_t *acpi_get_table(char *signature);