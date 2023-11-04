#pragma once
#include <misc/libc.h>
#include <devices/framebuffer/framebuffer.h>

#ifndef MODULE
#error logger has to have a MODULE name set
#endif

#define PREFIX_COLOUR 0xD0D0D0
#define DEBUG_COLOUR 0x3BADFF
#define INFO_COLOUR 0x10B010
#define WARN_COLOUR 0xD07000
#define ERROR_COLOUR 0xFF2020

static void log_info(const char *fmt, ...)
{
    va_list list;

    /// framebuffer
    if (framebuffer_available)
    {
        // print the prefix
        main_cursor.colour = PREFIX_COLOUR;
        printk("%s: ", MODULE);

        // print the format
        main_cursor.colour = INFO_COLOUR;
        va_start(list, fmt);
        vprintk(fmt, list);
        va_end(list);

        // new line
        printk("\n");
    }

    /// serial
    // print the prefix
    printk_serial("%s: ", MODULE);

    // print the format
    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);

    // new line
    printk_serial("\n");
}

static void log_warn(const char *fmt, ...)
{
    va_list list;

    /// framebuffer
    if (framebuffer_available)
    {
        // print the prefix
        main_cursor.colour = PREFIX_COLOUR;
        printk("%s: ", MODULE);

        // print the format
        main_cursor.colour = WARN_COLOUR;
        va_start(list, fmt);
        vprintk(fmt, list);
        va_end(list);

        // new line
        printk("\n");
    }

    /// serial
    // print the prefix
    printk_serial("%s: ", MODULE);

    // print the format
    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);

    // new line
    printk_serial("\n");
}

static void log_error(const char *fmt, ...)
{
    va_list list;

    /// framebuffer
    if (framebuffer_available)
    {
        // print the prefix
        main_cursor.colour = PREFIX_COLOUR;
        printk("%s: ", MODULE);

        // print the format
        main_cursor.colour = ERROR_COLOUR;

        va_start(list, fmt);
        vprintk(fmt, list);
        va_end(list);

        // new line
        printk("\n");
    }

    /// serial
    // print the prefix
    printk_serial("%s: ", MODULE);

    // print the format
    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);

    // new line
    printk_serial("\n");
}