#pragma once
#include <misc/libc.h>
#include <arch/x86_64/gdt/gdt.h>

typedef struct align_addr(16)
{
    // cpu identification
    uint64_t cpu_id;
    bool is_bsp;

    // internal tables
    gdt_info_t *gdt;
} arch_cpu_context_t;

arch_cpu_context_t *arch_context_install();
void *arch_install_scheduler_context(void *context);

arch_cpu_context_t *arch_get_cpu_context();
void *arch_get_scheduler_context();
