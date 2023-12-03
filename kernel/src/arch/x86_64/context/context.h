#pragma once
#include <misc/libc.h>
#include <arch/x86_64/gdt/gdt.h>

typedef struct
{
    // cpu identification
    uint16_t cpu_id;
    bool is_bsp;

    // internal tables
    gdt_info_t *gdt;

    // scheduler
} arch_context_t;

arch_context_t *arch_context_install();
extern arch_context_t *arch_get_context();