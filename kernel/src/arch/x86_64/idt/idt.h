#pragma once
#include <misc/libc.h>
#include <arch/x86_64/state/state.h>

pstruct
{
    uint16_t size;
    uint64_t offset;
}
arch_idtr_t;

extern arch_idtr_t arch_global_idtr;
extern void arch_idt_load(arch_idtr_t *);
void arch_interrupts_init(void);
void arch_interrupts_map_vector(uint64_t vector, void *handler);
uint16_t arch_interrupts_reserve_kernel_vector(void);

ifunc void arch_interrupts_enable(void)
{
    iasm("sti");
}

ifunc void arch_interrupts_disable(void)
{
    iasm("cli");
}