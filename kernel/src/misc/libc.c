#include <misc/libc.h>
#include <devices/framebuffer/framebuffer.h>
#include <devices/serial/serial.h>
#include <arch/arch.h>

size_t strlen(char *str)
{
    size_t i;
    for (i = 0; str[i]; i++)
        ;

    return i;
}

char *strcpy(char *dest, char *src)
{
    return memcpy(dest, src, strlen(src));
}

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }

    if (n == 0)
        return 0;
    else
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

void strrev(char *str)
{
    size_t len = strlen(str);
    for (int i = 0, j = len - 1; i < j; i++, j--)
    {
        const char a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

// safe itoa-like function
char *num_to_string(uint64_t value, char *buffer, size_t buffer_bytes, int base)
{
    buffer_bytes--; // reserve one byte for null-termination

    const char *digits = "0123456789ABCDEF";
    size_t buffer_index = 0;

    if (!value)
    {
        // "0"
        *buffer = '0';
        *(buffer + 1) = 0;
        return buffer;
    }

    // split the number in base
    while (value)
    {
        int digit = value % base;
        value /= base;
        buffer[buffer_index++] = digits[digit];

        if (buffer_index >= buffer_bytes) // safety check
            break;
    }

    // reverse the buffer
    for (int i = 0, j = buffer_index - 1; i < j; i++, j--)
    {
        const char a = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = a;
    }

    buffer[buffer_index] = 0; // null terminate buffer

    return buffer;
}

void printk(const char *fmt, ...)
{
    if (!framebuffer_available)
        return;

    va_list list;
    va_start(list, fmt);
    vprintk(fmt, list);
    va_end(list);
}

void printk_serial(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    vprintk_serial(fmt, list);
    va_end(list);
}

void printk_unsafe(const char *fmt, ...)
{
    if (!framebuffer_available)
        return;

    va_list list;
    va_start(list, fmt);
    vprintk_unsafe(fmt, list);
    va_end(list);
}

void printk_serial_unsafe(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    vprintk_serial_unsafe(fmt, list);
    va_end(list);
}

spinlock_t printk_lock;

void vprintk_unsafe(const char *fmt, va_list list)
{
    char conversion_buffer[32];
    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            framebuffer_write_character(fmt[i]);
            continue;
        }

        switch (fmt[i + 1])
        {
        case 'd': // decimal
            num_to_string(va_arg(list, uint64_t), conversion_buffer, 32, 10);
            framebuffer_write_string(conversion_buffer);
            break;
        case 'x': // hex
        case 'p': // pointer
            framebuffer_write_string("0x");
            num_to_string(va_arg(list, uint64_t), conversion_buffer, 32, 16);
            framebuffer_write_string(conversion_buffer);
            break;
        case 's':
            framebuffer_write_string(va_arg(list, char *)); // string
            break;
        case 'c':
            framebuffer_write_character(va_arg(list, int)); // char
            break;
        default:
            continue;
        }

        i++;
    }
}

void vprintk(const char *fmt, va_list list)
{
    spinlock_acquire(&printk_lock);
    vprintk_unsafe(fmt, list);
    spinlock_release(&printk_lock);
}

void vprintk_serial_unsafe(const char *fmt, va_list list)
{
    char conversion_buffer[32];

    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            serial_send_byte(fmt[i]);
            continue;
        }

        switch (fmt[i + 1])
        {
        case 'd': // decimal
            num_to_string(va_arg(list, uint64_t), conversion_buffer, 32, 10);
            serial_send_string(conversion_buffer);
            break;
        case 'x': // hex
        case 'p': // pointer
            serial_send_string("0x");
            num_to_string(va_arg(list, uint64_t), conversion_buffer, 32, 16);
            serial_send_string(conversion_buffer);
            break;
        case 's':
            serial_send_string(va_arg(list, char *)); // string
            break;
        case 'c':
            serial_send_byte(va_arg(list, int)); // char
            break;
        default:
            continue;
        }

        i++;
    }
}

void vprintk_serial(const char *fmt, va_list list)
{
    spinlock_acquire(&printk_lock);
    vprintk_serial_unsafe(fmt, list);
    spinlock_release(&printk_lock);
}

noreturn void halt()
{
    iasm("cli");
    iasm("hlt");
    while (1)
        ;
}