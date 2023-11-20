#include <misc/panic.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/page_table_manager/table_manager.h>
#include <arch/x86_64/xapic/xapic.h>
#include <arch/x86_64/idt/idt.h>
#include <arch/x86_64/timers/hpet.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

void arch_init()
{
    arch_load_gdt();
    arch_interrupts_init();
    arch_table_manager_init();
    arch_xapic_init(true);
}

void arch_late_init()
{
    arch_hpet_init();
}