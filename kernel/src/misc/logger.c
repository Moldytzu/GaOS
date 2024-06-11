#define MODULE "logger"
#include <misc/logger.h>

spinlock_t logger_spinlock;

void _log(uint32_t colour, const char *fmt, ...)
{
    spinlock_acquire(&logger_spinlock);
    va_list list;

    // read nanoseconds from system timer
    uint64_t nanoseconds = 0, miliseconds;
    if (clock_system_timer.time_keeping_capable)
        nanoseconds = clock_system_timer.read_nanoseconds();

    miliseconds = nanoseconds / 1000000 /*nanoseconds to miliseconds*/;

    /// framebuffer
    if (framebuffer_available)
    {
        // print the prefix
        main_cursor.colour = PREFIX_COLOUR;
        printk("[%d.%d%d%d] %s: ", miliseconds / 1000, miliseconds % 1000 / 100, miliseconds % 100 / 10, miliseconds % 10, MODULE);

        // print the format
        main_cursor.colour = colour;
        va_start(list, fmt);
        vprintk(fmt, list);
        va_end(list);

        // new line
        printk("\n");
    }

    /// serial
    // print the prefix
    printk_serial("[%d.%d%d%d] %s: ", miliseconds / 1000, miliseconds % 1000 / 100, miliseconds % 100 / 10, miliseconds % 10, MODULE);

    // print the format
    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);

    // new line
    printk_serial("\n");
    spinlock_release(&logger_spinlock);
}
