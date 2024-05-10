#include <misc/panic.h>
#include <arch/arch.h>
#include <arch/x86_64/context/context.h>
#include <arch/x86_64/msr.h>
#include <memory/physical/page_allocator.h>

#define MSR_GS_BASE 0xC0000101
#define MSR_GS_KERNEL_BASE 0xC0000102
#define CONTEXT_TYPE_CPU 0

void *arch_get_current_context()
{
    return (void *)rdmsr(MSR_GS_BASE);
}

arch_cpu_context_t *arch_get_cpu_context()
{
    arch_cpu_context_t *context = arch_get_current_context();
    if (context == nullptr || context->context_type != CONTEXT_TYPE_CPU)
        iasm("swapgs");

    return arch_get_current_context();
}

arch_cpu_context_t *arch_context_install()
{
    if (sizeof(arch_cpu_context_t) > PAGE)
        panic("arch context too big");

    arch_cpu_context_t *context = page_allocate(1);
    context->syscall_stack_top = (uint64_t)page_allocate(1) + PAGE;
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

void *arch_get_scheduler_context()
{
    return arch_get_cpu_context()->scheduler_context;
}