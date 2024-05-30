#define MODULE "main"
#include <misc/logger.h>
#include <misc/panic.h>

#include <misc/libc.h>
#include <boot/limine.h>

// acpi
#include <acpi/acpi.h>

// architecture
#include <arch/arch.h>

// devices
#include <devices/manager.h>
#include <devices/serial/serial.h>
#include <devices/framebuffer/framebuffer.h>
#include <devices/timers/timers.h>
#include <clock/clock.h>

// io
#include <io/queue.h>

// filesystem
#include <filesystem/vfs.h>
#include <filesystem/ustar.h>

// executables
#include <executables/elf.h>

// memory
#include <memory/physical/page_allocator.h>
#include <memory/physical/block_allocator.h>

// scheduling
#include <schedulers/task/task.h>

// this is the entry point of the kernel
void _start()
{
    arch_simd_enable();                                         // enable simd on this platform
    serial_init();                                              // initialise serial interface
    limine_init();                                              // initialise limine protocol
    framebuffer_init();                                         // initialise the framebuffer
    arch_early_init();                                          // initialise first arch stage
    page_allocator_init();                                      // initialise the page allocator
    arch_swap_stack(page_allocate(1), 1 * PAGE);                // allocate a new stack
    acpi_init();                                                // initialise the acpi
    arch_init();                                                // initialise second arch stage
    timers_init();                                              // initialise the timers
    block_allocator_init();                                     // initialise the block allocator
    log_info("bootstraped %d processors", arch_bootstrap_ap()); // boot up application processors
    arch_late_init();                                           // initialise last arch stage
    vfs_init();                                                 // initialise the virtual filesystem
    device_manager_init();                                      // initialise the device manager
    ustar_init();                                               // initialise the initrd
    task_scheduler_init();                                      // initialise the task scheduler
    io_queue_init();                                            // initialise the i/o queuer

    // load an executable from the initrd
    vfs_fs_node_t *hello_node = vfs_open("/initrd/hello.elf", O_RDONLY);
    if (is_error(hello_node))
        panic("failed to open hello.elf from initrd: %d", -error_of(hello_node));

    elf_load_from(hello_node);

    arch_bootstrap_ap_scheduler(); // let the application processors use the schedulers
    task_scheduler_enable();       // enable the task scheduler

    halt();
}