#include <boot/limine.h> // kernel interface

LIMINE_BASE_REVISION(1) // set latest revision

volatile struct limine_framebuffer_request kernel_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

void limine_init()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false) // check if the bootloader supports what we request
        halt();

    if (kernel_framebuffer_request.response == NULL || kernel_framebuffer_request.response->framebuffer_count < 1) // make sure we have a framebuffer
        halt();
}