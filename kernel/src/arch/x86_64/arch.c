#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>

void arch_init()
{
    arch_load_gdt();
}