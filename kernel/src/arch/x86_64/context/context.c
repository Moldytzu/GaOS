#include <misc/panic.h>
#include <arch/arch.h>
#include <arch/x86_64/context/context.h>
#include <arch/x86_64/msr.h>
#include <memory/physical/page_allocator.h>

#define MSR_GS_BASE 0xC0000101
#define MSR_GS_KERNEL_BASE 0xC0000102
#define CONTEXT_TYPE_SCHEDULER 1ULL

void *arch_get_current_context(void)
{
    return (void *)rdmsr(MSR_GS_BASE);
}

arch_cpu_context_t *arch_get_cpu_context(void)
{
    arch_cpu_context_t *context = arch_get_current_context();
    if (context == NULL || (uint64_t)context & CONTEXT_TYPE_SCHEDULER) // first bit in context address describes the type
        iasm("swapgs");

    return arch_get_current_context();
}

arch_cpu_context_t *arch_context_install(void)
{
    // gdt is a requirement for this

    if (sizeof(arch_cpu_context_t) > PAGE)
        panic("arch context too big");

    arch_cpu_context_t *context = page_allocate(1);
    wrmsr(MSR_GS_BASE, (uint64_t)context); // kernel context
    wrmsr(MSR_GS_KERNEL_BASE, 0);          // scheduler context

    return context;
}

void *arch_install_scheduler_context(void *context)
{
    arch_get_cpu_context(); // this makes the MSR_GS_KERNEL_BASE empty
    wrmsr(MSR_GS_KERNEL_BASE, (uint64_t)context | CONTEXT_TYPE_SCHEDULER);
    return context;
}

void *arch_get_scheduler_context(void)
{
    void *context = arch_get_current_context();
    if (!context)
        return NULL;

    if (((uint64_t)context & CONTEXT_TYPE_SCHEDULER) == 0)
    {
        iasm("swapgs");
        context = arch_get_current_context();
    }

    return (void *)((uint64_t)context & ~CONTEXT_TYPE_SCHEDULER); // remove the scheduler bit
}