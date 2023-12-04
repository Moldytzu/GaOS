#define MODULE "x86_64/smp"
#include <misc/logger.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/gdt/gdt.h>
#include <arch/x86_64/xapic/xapic.h>
#include <acpi/acpi.h>
#include <misc/libc.h>
#include <boot/limine.h>
#include <memory/physical/page_allocator.h>
#include <schedulers/task/task.h>

size_t arch_processor_count, arch_bsp_id;
bool arch_aps_online;

bool arch_is_bsp()
{
    return arch_get_id() == arch_bsp_id;
}

arch_spinlock_t arch_smp_bootstrap_lock, arch_smp_enable_scheduler_lock;
void arch_bootstrap_entry_limine(struct limine_smp_info *smp_info)
{
    (void)smp_info;

    arch_load_gdt();
    arch_table_manager_switch_to(arch_bootstrap_page_table);
    arch_swap_stack(page_allocate(1), PAGE);
    arch_xapic_init(true);
    arch_idt_load(&arch_global_idtr);

    arch_spinlock_release(&arch_smp_bootstrap_lock); // release the lock to indicate that we're ready

    // spin until the schedulers are ready to use
    while (arch_smp_enable_scheduler_lock)
        arch_hint_spinlock();

    task_scheduler_install_context();

    halt();
}

int arch_bootstrap_ap_limine()
{
    arch_spinlock_acquire(&arch_smp_enable_scheduler_lock);

    arch_processor_count = kernel_smp_request.response->cpu_count;
    arch_bsp_id = kernel_smp_request.response->bsp_lapic_id;

    struct limine_smp_info **cpus = kernel_smp_request.response->cpus;
    for (size_t i = 0; i < arch_processor_count; i++)
    {
        if (cpus[i]->lapic_id == arch_bsp_id) // ignore the bsp (this processor)
            continue;

        arch_spinlock_acquire(&arch_smp_bootstrap_lock); // lock

        cpus[i]->goto_address = arch_bootstrap_entry_limine; // wake up the processor

        while (arch_smp_bootstrap_lock) // wait to be unlocked by the processor
            arch_hint_spinlock();
    }

    arch_aps_online = arch_processor_count > 1; // mark application processors as online if there are some

    return arch_processor_count - 1;
}

int arch_bootstrap_ap()
{
    return arch_bootstrap_ap_limine();
}

void arch_bootstrap_ap_scheduler()
{
    arch_spinlock_release(&arch_smp_enable_scheduler_lock);
}