#define MODULE "sys_fork"
#include <misc/logger.h>

#include <arch/arch.h>
#include <syscalls/helpers.h>
#include <memory/physical/page_allocator.h>
#include <arch/x86_64/page_table_manager/table_manager.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/fork.html

int64_t sys_fork(arch_cpu_state_t *state)
{
    // this is a special syscall because it doesn't follow the standard way of implementing a syscall
    // it requires to have the full cpu state at the syscall entry so it can perfectly duplicate it

    scheduler_task_t *parent = GET_CALLER_TASK();
    scheduler_task_t *child = task_scheduler_create(parent->name);

    // duplicate the simd state
    arch_save_simd_state(&child->simd_state);

    // duplicate state
    memcpy(&child->state, state, sizeof(arch_cpu_state_t));

#ifdef ARCH_x86_64
    child->state.rsp = parent->userspace_stack_top;
    child->state.rip = state->rcx;
    child->state.rflags = state->r11 | 0x200;
    child->state.rax = 0;
    child->state.rcx = 0;
    child->state.r11 = 0;
    child->state.cs = 8 * 4 | 3;
    child->state.ss = 8 * 3 | 3;

    // duplicate the working set
    // fixme: create an allocator that handles this for us
    //        for now we go thru the whole page table to find present pages to reallocate and clone
    //        this is a very bad approach
    arch_page_table_t *child_pt = arch_table_manager_new(), *parent_pt = (arch_page_table_t *)(state->cr3 + kernel_hhdm_offset);

    for (int i = 0x100; i <= 0x1FF; i++) // map the higher half
        child_pt->entries[i] = arch_bootstrap_page_table->entries[i];

    for (uint64_t i1 = 0; i1 < 0x100; i1++)
    {
        uint64_t e1 = parent_pt->entries[i1];
        arch_page_table_layer_t *l1 = ((arch_page_table_layer_t *)(arch_table_manager_get_address(&e1) + kernel_hhdm_offset));
        if (e1 & TABLE_ENTRY_PRESENT)
        {
            for (uint64_t i2 = 0; i2 < 0x200; i2++)
            {
                uint64_t e2 = l1->entries[i2];
                arch_page_table_layer_t *l2 = ((arch_page_table_layer_t *)(arch_table_manager_get_address(&e2) + kernel_hhdm_offset));
                if (e2 & TABLE_ENTRY_PRESENT)
                {
                    for (uint64_t i3 = 0; i3 < 0x200; i3++)
                    {
                        uint64_t e3 = l2->entries[i3];
                        arch_page_table_layer_t *l3 = ((arch_page_table_layer_t *)(arch_table_manager_get_address(&e3) + kernel_hhdm_offset));
                        if (e3 & TABLE_ENTRY_PRESENT)
                        {
                            for (uint64_t i4 = 0; i4 < 0x200; i4++)
                            {
                                uint64_t e4 = l3->entries[i4];
                                if (e4 & TABLE_ENTRY_PRESENT)
                                {
                                    uint64_t parent_physical_address = arch_table_manager_get_address(&e4) + kernel_hhdm_offset;
                                    uint64_t child_physical_address = (uint64_t)page_allocate(1);
                                    uint64_t virtual_address = i4 << 12 | i3 << 21 | i2 << 30 | i1 << 39;

                                    memcpy((void *)child_physical_address, (void *)parent_physical_address, PAGE);

                                    arch_table_manager_map(child_pt, virtual_address, child_physical_address - kernel_hhdm_offset, e4 & 0xFFF0000000000FFFULL);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    child->state.cr3 = (uint64_t)child_pt - kernel_hhdm_offset;
#endif

    // duplicate the fd table
    child->fd_count = parent->fd_count;
    child->fd_max = parent->fd_max;
    child->fd_allocated_pages = parent->fd_allocated_pages;
    child->fd_translation = page_allocate(child->fd_allocated_pages);

    // fixme: here we have to call the filesystem instead to duplicate the file descriptor for us
    for (size_t i = 0; i < child->fd_count; i++)
        child->fd_translation[i] = parent->fd_translation[i];

    child->empty = false;

    return child->id;
}