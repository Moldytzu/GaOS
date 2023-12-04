#pragma once
#include <misc/libc.h>
#include <arch/x86_64/gdt/gdt.h>

typedef struct
{
    bool type; // false for cpu, true for scheduler

    // cpu identification
    uint16_t cpu_id;
    bool is_bsp;

    // internal tables
    gdt_info_t *gdt;

    // scheduler
} arch_cpu_context_t;

arch_cpu_context_t *arch_context_install();
void *arch_install_scheduler_context(void *context);

arch_cpu_context_t *arch_get_cpu_context();
void *arch_get_scheduler_context();
