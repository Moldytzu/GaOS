#define MODULE "x86_64/smp"
#include <misc/logger.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/xapic/xapic.h>
#include <acpi/acpi.h>
#include <misc/libc.h>
#include <boot/limine.h>
#include <memory/physical/page_allocator.h>

size_t arch_processor_count, arch_bsp_id;

bool arch_is_bsp()
{
    return arch_get_id() == arch_bsp_id;
}

void arch_bootstrap_entry_limine(struct limine_smp_info *smp_info)
{
    (void)smp_info;

    arch_load_gdt();
    arch_table_manager_switch_to(arch_bootstrap_page_table);
    arch_swap_stack(page_allocate(1), PAGE);
    arch_xapic_init(true);

    halt();
}

int arch_bootstrap_ap_limine()
{
    arch_processor_count = kernel_smp_request.response->cpu_count;
    arch_bsp_id = kernel_smp_request.response->bsp_lapic_id;

    struct limine_smp_info **cpus = kernel_smp_request.response->cpus;
    for (size_t i = 0; i < arch_processor_count; i++)
    {
        if (cpus[i]->lapic_id == arch_bsp_id) // ignore the bsp (this processor)
            continue;

        cpus[i]->goto_address = arch_bootstrap_entry_limine; // wake up the processor
    }

    return arch_processor_count - 1;
}

int arch_bootstrap_ap()
{
    return arch_bootstrap_ap_limine();
}