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

// convert to a string (base 10)
char to_stringout[32];
arch_spinlock_t to_string_lock;
char *to_string(uint64_t val)
{
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    arch_spinlock_acquire(&to_string_lock);

    int i = 0;
    for (; val; i++, val /= 10)
        to_stringout[i] = (val % 10) + '0';

    to_stringout[i] = 0; // terminate string

    strrev(to_stringout); // reverse string

    arch_spinlock_release(&to_string_lock);

    return to_stringout;
}

// convert to a string (base 16)
char to_hstringout[32];
char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    arch_spinlock_acquire(&to_string_lock);

    int i = 0;
    for (; i < 16 && val; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];  // get each nibble

    to_hstringout[i] = 0; // terminate string

    strrev(to_hstringout); // reverse string

    arch_spinlock_release(&to_string_lock);

    return to_hstringout; // return the string
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

void vprintk(const char *fmt, va_list list)
{
    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            framebuffer_write_character(fmt[i]);
            continue;
        }

        switch (fmt[i + 1])
        {
        case 'd':
            framebuffer_write_string(to_string(va_arg(list, uint64_t))); // decimal
            break;
        case 'x':
        case 'p':
            framebuffer_write_string(to_hstring((uint64_t)va_arg(list, void *))); // pointer/hex
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

void vprintk_serial(const char *fmt, va_list list)
{
    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            serial_send_byte(fmt[i]);
            continue;
        }

        switch (fmt[i + 1])
        {
        case 'd':
            serial_send_string(to_string(va_arg(list, uint64_t))); // decimal
            break;
        case 'x':
        case 'p':
            serial_send_string(to_hstring((uint64_t)va_arg(list, void *))); // pointer/hex
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

noreturn void halt()
{
    iasm("cli");
    iasm("hlt");
    while (1)
        ;
}