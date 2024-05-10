#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <config.h>
#include <lock/spinlock.h>

// fixme: as soon as GCC supports C++-style attributes for C23, use them here

#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define bitsof(type) (sizeof(type) * 8)
#define pstruct typedef struct __attribute__((packed))
#define align_addr(al) __attribute__((aligned(al)))
#define used(x) (void)x

void *memcpy(void *dest, const void *src, size_t n);
char *strcpy(char *dest, char *src);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(char *str);
void strrev(char *str);

void vprintk(const char *fmt, va_list list);
void vprintk_serial(const char *fmt, va_list list);
void printk(const char *fmt, ...);
void printk_serial(const char *fmt, ...);

noreturn void halt();

ifunc void zero64(uint64_t *s, size_t n)
{
    n /= sizeof(uint64_t);
    for (size_t i = 0; i < n; i++)
        s[i] = 0;
}

ifunc int min(int a, int b)
{
    if (a > b)
        return b;
    return a;
}

ifunc int max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

ifunc int abs(int a)
{
    if (a < 0)
        return -a;
    return a;
}