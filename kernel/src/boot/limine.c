#include <boot/limine.h> // kernel interface

LIMINE_BASE_REVISION(1) // set latest revision

volatile struct limine_framebuffer_request kernel_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

volatile struct limine_hhdm_request kernel_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

volatile struct limine_memmap_request kernel_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

volatile struct limine_kernel_address_request kernel_kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

volatile struct limine_stack_size_request kernel_stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 0x1000, /*a page*/
};

volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

uint64_t kernel_hhdm_offset;

void limine_init()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false) // check if the bootloader supports what we request
        halt();

    if (kernel_framebuffer_request.response == NULL || kernel_framebuffer_request.response->framebuffer_count < 1) // make sure we have a framebuffer
        halt();

    kernel_hhdm_offset = kernel_hhdm_request.response->offset;
}