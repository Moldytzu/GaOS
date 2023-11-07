#include <misc/panic.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/page_table_manager/table_manager.h>
#include <arch/x86_64/xapic/xapic.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

uint64_t arch_trampoline_base;

void arch_init()
{
    arch_trampoline_base = (uint64_t)page_allocate(1) - kernel_hhdm_offset;
    if (arch_trampoline_base + PAGE > 1 * 1024 * 1024)
        panic("Too high trampoline base. 0x%p > 1 MB", arch_trampoline_base);

    arch_load_gdt();
    arch_table_manager_init();
    arch_xapic_init(true);
}