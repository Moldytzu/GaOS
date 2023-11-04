#pragma once
#include <misc/libc.h>

void page_allocator_init();
void *page_allocate(size_t pages);
void page_deallocate(void *base, size_t pages);