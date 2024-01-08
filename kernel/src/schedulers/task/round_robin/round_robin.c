#include <schedulers/task/round_robin/round_robin.h>
#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <devices/serial/serial.h>
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

typedef struct
{
    scheduler_task_t *current;
    scheduler_task_t *head;
    scheduler_task_t *last;
    arch_spinlock_t lock;
} scheduler_task_queue_t;

struct scheduler_context
{
    uint64_t cpu_id;

    scheduler_task_queue_t running;

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

scheduler_task_t *task_scheduler_round_robin_pop_from_queue(scheduler_task_queue_t *queue)
{
    // arch_spinlock_acquire(&queue->lock);
    scheduler_task_t *to_pop = queue->head; // grab our candidate off the queue

    if (queue->head != queue->last && to_pop) // if it isn't alone and it is valid we can move it in the back
    {
        queue->head = queue->head->next; // move the head after it
        queue->last->next = to_pop;      // insert it in the very end
        queue->last = to_pop;            // make it the end
        to_pop->next = queue->head;      // link it to the start
    }
    queue->current = to_pop;
    arch_spinlock_release(&queue->lock);
    return to_pop;
}

void task_scheduler_round_robin_push_to_queue(scheduler_task_queue_t *queue, scheduler_task_t *task)
{
    arch_spinlock_acquire(&queue->lock);

    // push it in the back
    if (queue->last)
    {
        queue->last->next = task;
        task->previous = queue->last;
        queue->last = task;
    }
    else
        queue->head = queue->last = task;

    arch_spinlock_release(&queue->lock);
}

void task1()
{
    while (true)
        serial_send_byte('A');

    task1();
}
void task2()
{
    while (true)
        serial_send_byte('B');

    task2();
}
void task3()
{
    while (true)
        serial_send_byte('C');

    task3();
}

noreturn void task_scheduler_round_robin_reschedule(arch_cpu_state_t *state)
{
    // todo: save simd state here

    // get our context
    scheduler_context_t *context = (scheduler_context_t *)arch_get_scheduler_context();

    // save current state
    scheduler_task_t *current = context->running.current;
    memcpy(&current->state, state, sizeof(arch_cpu_state_t));

    printk("saving %d\n", current->id);

    // load new state
    scheduler_task_t *next_task = task_scheduler_round_robin_pop_from_queue(&context->running);
    printk("loading %d\n", next_task->id);
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
    scheduler_context_t *context = (scheduler_context_t *)arch_get_scheduler_context();

#ifdef ARCH_x86_64
    arch_cpu_state_t state;
    memset(&state, 0, sizeof(arch_cpu_state_t));
    state.cr3 = (uint64_t)arch_bootstrap_page_table - kernel_hhdm_offset;
    state.cs = 8 * 2 | 0; //
    state.rflags = 0x202;
    state.ss = 8 * 1 | 0; //

    scheduler_task_t *task = block_allocate(sizeof(scheduler_task_t)), *first;
    first = task;
    task->id = 1;
    task->state = state;
    task->state.rip = (uint64_t)task1;
    task->state.rsp = (uint64_t)page_allocate(1) + PAGE;
    task_scheduler_round_robin_push_to_queue(&context->running, task);

    task = block_allocate(sizeof(scheduler_task_t));
    task->id = 2;
    task->state = state;
    task->state.rip = (uint64_t)task2;
    task->state.rsp = (uint64_t)page_allocate(1) + PAGE;
    task_scheduler_round_robin_push_to_queue(&context->running, task);

    task = block_allocate(sizeof(scheduler_task_t));
    task->id = 3;
    task->state = state;
    task->state.rip = (uint64_t)task3;
    task->state.rsp = (uint64_t)page_allocate(1) + PAGE;
    task_scheduler_round_robin_push_to_queue(&context->running, task);

    context->running.current = first;
    clock_preemption_timer.schedule_one_shot();
    arch_interrupts_enable();
    task1();
#endif
}