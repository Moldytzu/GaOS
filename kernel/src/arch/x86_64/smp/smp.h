#pragma once
#include <misc/libc.h>

extern size_t arch_processor_count;
extern bool arch_aps_online;

int arch_bootstrap_ap();
bool arch_is_bsp();