#include <misc/libc.h>
#include <boot/limine.h>

// architecture
#include <arch/arch.h>

// devices
#include <devices/serial/serial.h>
#include <devices/framebuffer/framebuffer.h>

// this is the entry point of the kernel
void _start(void)
{
    arch_simd_enable(); // enable simd on this platform
    serial_init();      // initialise serial interface
    limine_init();      // initialise limine protocol
    framebuffer_init(); // initialise the framebuffer

    printk("this should be on framebuffer %d %x", 10, 10);
    printk_serial("this should be on serial %d %x", 10, 10);

    halt();
}