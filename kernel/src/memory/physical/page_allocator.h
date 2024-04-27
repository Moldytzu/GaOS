#pragma once
#include <misc/libc.h>

#ifndef PAGE
#include <arch/arch.h>
#endif

void page_allocator_init(void);
void *page_allocate(size_t pages);
void page_deallocate(void *base, size_t pages);