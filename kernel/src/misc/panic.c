#include <misc/panic.h>
#include <misc/libc.h>
#include <devices/framebuffer/framebuffer.h>
#include <arch/arch.h>

noreturn void panic(const char *fmt, ...)
{
    arch_kill_ap();

    va_list list;

    /// framebuffer
    if (framebuffer_available)
    {
        main_cursor.colour = 0xFF0000;
        printk("Kernel panic.\nMessage: ");

        va_start(list, fmt);
        vprintk(fmt, list);
        va_end(list);
    }

    /// serial
    printk_serial("Kernel panic.\nMessage: ");

    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);

    halt();
}