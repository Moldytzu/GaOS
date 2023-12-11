#include <schedulers/task/round_robin/round_robin.h>
#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <arch/arch.h>

typedef struct
{
    uint64_t id;

    arch_cpu_state_t state align_addr(16);
} scheduler_task_t;

struct scheduler_context
{
    uint64_t cpu_id;

    struct scheduler_context *next;
    struct scheduler_context *previous;
};

typedef struct scheduler_context scheduler_context_t;

scheduler_context_t *last_context = NULL;
arch_spinlock_t last_context_lock;

void task_scheduler_round_robin_install_context()
{
    scheduler_context_t *new_context = block_allocate(sizeof(scheduler_context_t));
    new_context->cpu_id = arch_get_id();

    arch_spinlock_acquire(&last_context_lock);
    if (last_context)
    {
        last_context->next = new_context;
        new_context->previous = last_context;
    }

    last_context = new_context;
    arch_spinlock_release(&last_context_lock);

    arch_install_scheduler_context(new_context);
}

void test_task()
{
    while (true)
        ;
}

void task_scheduler_round_robin_init()
{
    /*
#ifdef ARCH_x86_64
    arch_cpu_state_t state;
    memset(&state, 0, sizeof(arch_cpu_state_t));
    state.cr3 = (uint64_t)arch_bootstrap_page_table - kernel_hhdm_offset;
    state.rip = (uint64_t)test_task;
    state.cs = 8 * 4 | 3; // 8*2|0
    state.rflags = 0x202;
    state.rsp = (uint64_t)page_allocate(1) + PAGE;
    state.ss = 8 * 3 | 3; // 8*1|0
    arch_switch_state(&state);
#endif
    */

    task_scheduler_round_robin_install_context();
}