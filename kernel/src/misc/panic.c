#include <misc/panic.h>
#include <misc/libc.h>
#include <devices/framebuffer/framebuffer.h>
#include <arch/arch.h>

// displays the address if possible
#define PRINT_TRACE_IF_POSSIBLE(x)                           \
    if ((uint64_t)STACK_TRACE_WALK(x) <= 0xFFFFFFFF80000000) \
        halt();                                              \
    printk_unsafe("Trace %d: %p\n", x, STACK_TRACE_WALK(x)); \
    printk_serial_unsafe("Trace %d: %p\n", x, STACK_TRACE_WALK(x));

noreturn void panic(const char *fmt, ...)
{
    arch_kill_ap();
    arch_interrupts_disable();

    va_list list;

    /// framebuffer
    if (framebuffer_available)
    {
        main_cursor.colour = 0xFF0000;
        printk_unsafe("Kernel panic.\nMessage: ");

        va_start(list, fmt);
        vprintk_unsafe(fmt, list);
        va_end(list);
    }

    /// serial
    printk_serial_unsafe("Kernel panic.\nMessage: ");

    va_start(list, fmt);
    vprintk_serial_unsafe(fmt, list);
    va_end(list);

    /// print stack trace
    printk_unsafe("\n");
    printk_serial_unsafe("\n");
    PRINT_TRACE_IF_POSSIBLE(1);
    PRINT_TRACE_IF_POSSIBLE(2);
    PRINT_TRACE_IF_POSSIBLE(3);
    PRINT_TRACE_IF_POSSIBLE(4);
    PRINT_TRACE_IF_POSSIBLE(5);
    PRINT_TRACE_IF_POSSIBLE(6);

    halt();
}