#include <schedulers/task/round_robin/round_robin.h>
#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <clock/clock.h>
#include <arch/arch.h>

struct scheduler_task
{
    uint64_t id;

    arch_cpu_state_t state align_addr(16);

    struct scheduler_task *next;
    struct scheduler_task *previous;
};

typedef struct scheduler_task scheduler_task_t;

struct scheduler_context
{
    uint64_t cpu_id;

    scheduler_task_t *running_current;
    scheduler_task_t *running_head;
    scheduler_task_t *running_last;
    arch_spinlock_t running_lock;

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

scheduler_task_t *task_scheduler_round_robin_pop_from_running_list()
{
    scheduler_context_t *context = arch_get_scheduler_context();

    // arch_spinlock_acquire(&context->running_lock);
    scheduler_task_t *to_pop = context->running_head; // grab our candidate off the list

    if (context->running_head != context->running_last && to_pop) // if it isn't alone and it is valid we can move it in the back
    {
        context->running_head = context->running_head->next; // move the head after it
        context->running_last->next = to_pop;                // insert it in the very end
        context->running_last = to_pop;                      // make it the end
        to_pop->next = context->running_head;                // link it to the start
    }
    context->running_current = to_pop;
    arch_spinlock_release(&context->running_lock);
    return to_pop;
}

void task_scheduler_round_robin_push_to_running_list(scheduler_task_t *task)
{
    scheduler_context_t *context = arch_get_scheduler_context();
    arch_spinlock_acquire(&context->running_lock);

    // push it in the back
    if (context->running_last)
    {
        context->running_last->next = task;
        task->previous = context->running_last;
        context->running_last = task;
    }
    else
        context->running_head = context->running_last = task;

    arch_spinlock_release(&context->running_lock);
}

void task1()
{
    while (true)
        printk_serial("%d(%d)\n", 1, arch_get_id());
}
void task2()
{
    while (true)
        printk_serial("%d(%d)\n", 2, arch_get_id());
}
void task3()
{
    while (true)
        printk_serial("%d(%d)\n", 3, arch_get_id());
}

noreturn void task_scheduler_round_robin_reschedule(arch_cpu_state_t *state)
{
    // todo: save simd state here

    // save current state
    scheduler_task_t *current = ((scheduler_context_t *)arch_get_scheduler_context())->running_current;
    memcpy(&current->state, state, sizeof(arch_cpu_state_t));

    // load new state
    scheduler_task_t *next_task = task_scheduler_round_robin_pop_from_running_list();
    clock_preemption_timer.schedule_one_shot();
    arch_switch_state(&next_task->state);
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

#ifdef ARCH_x86_64
    arch_cpu_state_t state;
    memset(&state, 0, sizeof(arch_cpu_state_t));
    state.cr3 = (uint64_t)arch_bootstrap_page_table - kernel_hhdm_offset;
    state.cs = 8 * 2 | 0; //
    state.rflags = 0x202;
    state.rsp = (uint64_t)page_allocate(1) + PAGE;
    state.ss = 8 * 1 | 0; //

    scheduler_task_t *task = block_allocate(sizeof(scheduler_task_t)), *first;
    first = task;
    task->id = 1;
    task->state = state;
    task->state.rip = (uint64_t)task1;
    task_scheduler_round_robin_push_to_running_list(task);

    task = block_allocate(sizeof(scheduler_task_t));
    task->id = 2;
    task->state = state;
    task->state.rip = (uint64_t)task2;
    task_scheduler_round_robin_push_to_running_list(task);

    task = block_allocate(sizeof(scheduler_task_t));
    task->id = 3;
    task->state = state;
    task->state.rip = (uint64_t)task3;
    task_scheduler_round_robin_push_to_running_list(task);

    scheduler_context_t *context = arch_get_scheduler_context();
    context->running_current = first;
    clock_preemption_timer.schedule_one_shot();
    arch_interrupts_enable();
    task1();
#endif
}