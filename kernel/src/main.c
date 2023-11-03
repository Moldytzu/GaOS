#include <misc/libc.h>
#include <boot/limine.h>

// this is the entry point of the kernel
void _start(void)
{
    limine_init(); // initialise limine protocol

    struct limine_framebuffer *framebuffer = kernel_framebuffer_request.response->framebuffers[0];
    for (size_t i = 0; i < 100; i++)
    {
        volatile uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    halt();
}