#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/page_table_manager/table_manager.h>

void arch_init()
{
    arch_load_gdt();
    arch_table_manager_init(); // fixme: move this from here.....
}