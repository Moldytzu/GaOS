#pragma once
#include <misc/libc.h>
#include <arch/x86_64/gdt/gdt.h>

typedef struct align_addr(16)
{
    uint64_t context_type;
    uint64_t syscall_stack_top;
    uint64_t old_user_stack;

    // cpu identification
    uint64_t cpu_id;
    bool is_bsp;

    // internal tables
    gdt_info_t *gdt;
    void *scheduler_context;
}
arch_cpu_context_t;

arch_cpu_context_t *arch_context_install();
void *arch_install_scheduler_context(void *context);
void *arch_install_task_context(void *context);

arch_cpu_context_t *arch_get_cpu_context();
void *arch_get_scheduler_context();
void *arch_get_task_context();