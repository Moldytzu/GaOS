#include <misc/panic.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/page_table_manager/table_manager.h>
#include <arch/x86_64/xapic/xapic.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

void arch_init()
{
    arch_load_gdt();
    arch_table_manager_init();
    arch_xapic_init(true);
}