#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <config.h>

#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define bitsof(type) (sizeof(type) * 8)
#define pstruct typedef struct __attribute__((packed))
#define align_addr(al) __attribute__((aligned(al)))

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(char *str);

void vprintk(const char *fmt, va_list list);
void vprintk_serial(const char *fmt, va_list list);
void printk(const char *fmt, ...);
void printk_serial(const char *fmt, ...);

noreturn void halt();