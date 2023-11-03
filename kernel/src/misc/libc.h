#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#define iasm asm volatile

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

void halt();