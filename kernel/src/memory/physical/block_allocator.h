#pragma once
#include <misc/libc.h>

void block_allocator_init();
void *block_allocate(size_t size);
void block_deallocate(void *block);