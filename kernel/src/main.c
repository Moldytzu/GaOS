#define MODULE "main"
#include <misc/logger.h>

#include <misc/libc.h>
#include <boot/limine.h>

// acpi
#include <acpi/acpi.h>

// architecture
#include <arch/arch.h>

// devices
#include <devices/serial/serial.h>
#include <devices/framebuffer/framebuffer.h>

// memory
#include <memory/physical/page_allocator.h>

// this is the entry point of the kernel
void _start(void)
{
    arch_simd_enable();                                         // enable simd on this platform
    serial_init();                                              // initialise serial interface
    limine_init();                                              // initialise limine protocol
    framebuffer_init();                                         // initialise the framebuffer
    page_allocator_init();                                      // initialise the page allocator
    arch_swap_stack(page_allocate(1), PAGE);                    // allocate a new stack
    arch_init();                                                // initialise this platform
    acpi_init();                                                // initialise the acpi
    log_info("bootstraped %d processors", arch_bootstrap_ap()); // boot up application cores

    halt();
}