#include <misc/panic.h>
#include <arch/arch.h>
#include <arch/x86_64/context/context.h>
#include <arch/x86_64/msr.h>
#include <memory/physical/page_allocator.h>

#define MSR_GS_BASE 0xC0000101
#define MSR_GS_KERNEL_BASE 0xC0000102
#define CONTEXT_TYPE_CPU 0

void *arch_get_current_context(void)
{
    return (void *)rdmsr(MSR_GS_BASE);
}

arch_cpu_context_t *arch_get_cpu_context(void)
{
    arch_cpu_context_t *context = arch_get_current_context();
    if (context == NULL || context->context_type != CONTEXT_TYPE_CPU)
        iasm("swapgs");

    return arch_get_current_context();
}

arch_cpu_context_t *arch_context_install(void)
{
    if (sizeof(arch_cpu_context_t) > PAGE)
        panic("arch context too big");

    arch_cpu_context_t *context = page_allocate(1);
    wrmsr(MSR_GS_BASE, (uint64_t)context); // kernel context
    wrmsr(MSR_GS_KERNEL_BASE, 0);          // scheduler context

    return context;
}

void *arch_install_scheduler_context(void *context)
{
    arch_cpu_context_t *cpu_context = arch_get_cpu_context();
    cpu_context->scheduler_context = context;
    return context;
}

void *arch_get_scheduler_context(void)
{
    return arch_get_cpu_context()->scheduler_context;
}