#include <misc/panic.h>
#include <arch/arch.h>
#include <arch/x86_64/context/context.h>
#include <arch/x86_64/msr.h>
#include <memory/physical/page_allocator.h>

#define MSR_GS_BASE 0xC0000101
#define MSR_GS_KERNEL_BASE 0xC0000102

arch_context_t *arch_context_install()
{
    // gdt is a requirement for this

    if (sizeof(arch_context_t) > PAGE)
        panic("arch context too big");

    arch_context_t *context = page_allocate(1);
    wrmsr(MSR_GS_BASE, (uint64_t)context);        // kernel context
    wrmsr(MSR_GS_KERNEL_BASE, (uint64_t)context); // userspace context

    return context;
}