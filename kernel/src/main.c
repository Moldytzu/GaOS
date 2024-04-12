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
#include <devices/timers/timers.h>
#include <clock/clock.h>

// filesystem
#include <filesystem/vfs.h>

// memory
#include <memory/physical/page_allocator.h>
#include <memory/physical/block_allocator.h>

// scheduling
#include <schedulers/task/task.h>

// this is the entry point of the kernel
void _start(void)
{
    arch_simd_enable();                                         // enable simd on this platform
    serial_init();                                              // initialise serial interface
    limine_init();                                              // initialise limine protocol
    framebuffer_init();                                         // initialise the framebuffer
    arch_early_init();                                          // initialise first arch stage
    page_allocator_init();                                      // initialise the page allocator
    arch_swap_stack(page_allocate(1), PAGE);                    // allocate a new stack
    acpi_init();                                                // initialise the acpi
    arch_init();                                                // initialise second arch stage
    timers_init();                                              // initialise the timers
    block_allocator_init();                                     // initialise the block allocator
    log_info("bootstraped %d processors", arch_bootstrap_ap()); // boot up application processors
    arch_late_init();                                           // initialise last arch stage
    vfs_init();                                                 // initialise the virtual filesystem
    task_scheduler_init();                                      // initialise the task scheduler
    arch_bootstrap_ap_scheduler();                              // let the application processors use the schedulers

    halt();
}