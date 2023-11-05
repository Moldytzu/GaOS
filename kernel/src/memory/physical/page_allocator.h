#pragma once
#include <misc/libc.h>

#ifndef PAGE
#define PAGE (4096)
#endif

void page_allocator_init();
void *page_allocate(size_t pages);
void page_deallocate(void *base, size_t pages);