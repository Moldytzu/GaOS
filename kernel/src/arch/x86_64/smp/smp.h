#pragma once
#include <misc/libc.h>

extern size_t arch_processor_count;

int arch_bootstrap_ap();
bool arch_is_bsp();