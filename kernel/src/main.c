#include <misc/libc.h>
#include <boot/limine.h>

// architecture
#include <arch/arch.h>

// devices
#include <devices/serial/serial.h>

// this is the entry point of the kernel
void _start(void)
{
    arch_simd_enable(); // enable simd on this platform
    serial_init();      // initialise serial interface
    limine_init();      // initialise limine protocol

    // draw a line on screen
    struct limine_framebuffer *framebuffer = kernel_framebuffer_request.response->framebuffers[0];
    for (size_t i = 0; i < 100; i++)
    {
        volatile uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    halt();
}