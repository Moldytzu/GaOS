#include <schedulers/task/round_robin/round_robin.h>
#include <memory/physical/block_allocator.h>
#include <arch/arch.h>

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

void task_scheduler_round_robin_init()
{
    task_scheduler_round_robin_install_context();
}