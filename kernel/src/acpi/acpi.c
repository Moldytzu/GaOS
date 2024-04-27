#define MODULE "acpi"
#include <misc/logger.h>

#include <acpi/acpi.h>
#include <boot/limine.h>

pstruct
{
    char signature[8];
    uint8_t checksum;
    char oem[6];
    uint8_t revision;
    uint32_t rsdt;

    // acpi version 2.0+
    uint32_t length;
    uint64_t xsdt;
    uint8_t extended_checksum;
}
acpi_xsdp_t;

acpi_sdt_header_t *sdt;
acpi_xsdp_t *sdp;

acpi_sdt_header_t *acpi_get_table_xsdt(char *signature)
{
    size_t entries = (sdt->length - sizeof(acpi_sdt_header_t)) / sizeof(uint64_t);
    uint64_t *headers = (uint64_t *)((uint64_t)sdt + sizeof(acpi_sdt_header_t));

    for (size_t i = 0; i < entries; i++)
    {
        acpi_sdt_header_t *header = (acpi_sdt_header_t *)(headers[i] + kernel_hhdm_offset);
        if (memcmp(header->signature, signature, 4) == 0)
            return header;
    }

    log_error("failed to get SDT %s", signature);

    return NULL;
}

acpi_sdt_header_t *acpi_get_table_rsdt(char *signature)
{
    size_t entries = (sdt->length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
    uint32_t *headers = (uint32_t *)((uint64_t)sdt + sizeof(acpi_sdt_header_t));

    for (size_t i = 0; i < entries; i++)
    {
        acpi_sdt_header_t *header = (acpi_sdt_header_t *)((uint64_t)headers[i] + kernel_hhdm_offset);
        if (memcmp(header->signature, signature, 4) == 0)
            return header;
    }

    log_error("failed to get SDT %s", signature);

    return NULL;
}

acpi_sdt_header_t *acpi_get_table(char *signature)
{
    if (sdp->revision == 0) // acpi 1.0
        return acpi_get_table_rsdt(signature);
    else // acpi 2.0+
        return acpi_get_table_xsdt(signature);
}

uint8_t acpi_revision;

void acpi_init(void)
{
    sdp = (acpi_xsdp_t *)kernel_rsdp_request.response->address;

    log_info("revision %d", sdp->revision);

    acpi_revision = sdp->revision;

    // determine the correct system descriptor table based on revision
    if (sdp->revision == 0) // acpi 1.0
        sdt = (acpi_sdt_header_t *)((uint64_t)sdp->rsdt + kernel_hhdm_offset);
    else // acpi 2.0+
        sdt = (acpi_sdt_header_t *)(sdp->xsdt + kernel_hhdm_offset);
}