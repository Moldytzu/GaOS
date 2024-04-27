#pragma once
#include <misc/libc.h>

extern size_t arch_processor_count;
extern bool arch_aps_online;

void arch_bootstrap_ap_scheduler(void);
int arch_bootstrap_ap(void);
bool arch_is_bsp(void);