#define MODULE "round_robin"
#include <misc/logger.h>

#include <misc/panic.h>
#include <schedulers/task/round_robin/round_robin.h>
#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <devices/serial/serial.h>
#include <clock/clock.h>
#include <arch/arch.h>

scheduler_context_t *last_context = nullptr;
spinlock_t last_context_lock;
int last_id = 0;
spinlock_t last_id_lock;

/*

Operations with lists

*/

void debug_dump_queue()
{
    spinlock_acquire(&last_id_lock);
    scheduler_context_t *context = arch_get_scheduler_context();
    scheduler_task_queue_t *queue = &context->running;

    printk_serial("\tlisting for %d %d tasks\n", context->cpu_id, queue->count);
    scheduler_task_t *task = queue->head;

    do
    {
        if (!task)
            panic("running queue of %d has a corrupt task", arch_get_id());
        printk_serial("\t\tid %d, next is %p\n", task->id, task->next);
        task = task->next;

    } while (task && task != queue->head);

    spinlock_release(&last_id_lock);
}

scheduler_task_t *task_scheduler_round_robin_pop_from_queue(scheduler_task_queue_t *queue)
{
    spinlock_acquire(&queue->lock);

    scheduler_task_t *to_pop = queue->head; // grab our candidate off the queue

    if (to_pop == nullptr)
        panic("Failed to pop a nullptr task from %p of %d", queue, arch_get_id());

    if (queue->count > 1) // if it's possible to move it in the back (i.e. there are more than one), do it
    {
        queue->head = queue->head->next; // move the head after it, second task
        queue->last->next = to_pop;      // insert it in the very end
        queue->last = to_pop;            // make it the end
        to_pop->next = queue->head;      // link it to the start
        queue->current = to_pop;
    }

    // printk_serial("returning %p from %p", to_pop, queue);

    spinlock_release(&queue->lock);
    return to_pop;
}

void task_scheduler_round_robin_push_to_queue(scheduler_task_queue_t *queue, scheduler_task_t *task)
{
    spinlock_acquire(&queue->lock);

    task->parent_queue = queue;

    // push it in the back
    if (queue->count > 0)
    {
        queue->last->next = task;
        queue->last = task;
    }
    else
        queue->current = queue->head = queue->last = task;

    queue->count++;

    // printk_serial("pushing %p to %p (%d new count)\n", task, queue, queue->count);

    spinlock_release(&queue->lock);
}

scheduler_task_t *task_scheduler_round_robin_get_pid(uint64_t pid)
{
    if (!pid)
        return nullptr;

    /// make sure we don't preempt while iterating
    bool interrupts_were_enabled = arch_interrupts_enabled();
    if (interrupts_were_enabled)
        arch_interrupts_disable();

    // iterate over all contexts
    spinlock_acquire(&last_context_lock);
    scheduler_context_t *current_context = last_context;
    do
    {
        // iterate over all tasks in the context
        spinlock_acquire(&current_context->running.lock);
        scheduler_task_t *current_task = current_context->running.head;

        do
        {
            if (current_task->id == pid)
            {
                spinlock_release(&current_context->running.lock);
                spinlock_release(&last_context_lock);

                if (interrupts_were_enabled)
                    arch_interrupts_enable();

                return current_task;
            }

            current_task = current_task->next;

        } while (current_task && current_task != current_context->running.head);

        spinlock_release(&current_context->running.lock);

        current_context = current_context->previous; // we start with the last context, thus we have to iterate backwards
    } while (current_context && current_context != last_context);

    spinlock_release(&last_context_lock);

    if (interrupts_were_enabled)
        arch_interrupts_enable();
    return nullptr;
}

/*

Context installation

*/

noreturn void _idle_task()
{
    while (1)
    {
        arch_interrupts_wait();
        arch_hint_spinlock();
    }
}

void task_scheduler_round_robin_install_context()
{
    log_info("installing scheduler for %d", arch_get_id());
    // allocate a scheduler context
    scheduler_context_t *new_context = block_allocate(sizeof(scheduler_context_t));
    new_context->cpu_id = arch_get_id();

    // add it in the chain
    spinlock_acquire(&last_context_lock);
    if (last_context)
    {
        last_context->next = new_context;
        new_context->previous = last_context;
    }

    last_context = new_context;
    spinlock_release(&last_context_lock);

    // install the context
    arch_install_scheduler_context(new_context);

    // create the idle task
    scheduler_task_t *task = block_allocate(sizeof(scheduler_task_t));
    task->name = "idle";
    task->name_length = 4;
    task->empty = false;

#ifdef ARCH_x86_64
    task->state.cr3 = (uint64_t)arch_bootstrap_page_table - kernel_hhdm_offset; // cr3 has to be a physical address
    task->state.rip = (uint64_t)_idle_task;
    task->state.rflags = 0x202; // enable interrupts
    task->state.rsp = (uint64_t)page_allocate(1) + PAGE;

    // point to kernel-space segements
    task->state.cs = 8 * 1 | 0;
    task->state.ss = 8 * 2 | 0;
#endif

    task_scheduler_round_robin_push_to_queue(&new_context->running, task); // add it in the list

    debug_dump_queue();
}

/*

Scheduling

*/

noreturn void task_scheduler_round_robin_reschedule(arch_cpu_state_t *state)
{
    // get our internal context
    scheduler_context_t *context = (scheduler_context_t *)arch_get_scheduler_context();
    spinlock_acquire(&context->running.lock);
    scheduler_task_t *current = context->running.current;

    // save state
    arch_save_simd_state(&current->simd_state);
    memcpy(&current->state, state, sizeof(arch_cpu_state_t));

    // printk_serial("saved %s (%d) on %d\n", current->name, current->id, arch_get_id());

    // load new state
    spinlock_release(&context->running.lock);
    scheduler_task_t *next_task = task_scheduler_round_robin_pop_from_queue(&context->running);
    while (next_task->empty)
        next_task = task_scheduler_round_robin_pop_from_queue(&context->running);
    arch_install_task_context(next_task);

    // printk_serial("loading %s (%d) on %d\n", next_task->name, next_task->id, arch_get_id());
    //  debug_dump_queue();

    arch_restore_simd_state(&next_task->simd_state);
    clock_preemption_timer.schedule_one_shot();
    arch_switch_state(&next_task->state);
}

scheduler_task_t *task_scheduler_round_robin_create(const char *name)
{
    scheduler_task_t *task = block_allocate(sizeof(scheduler_task_t));

    // allocate and copy name
    size_t name_len = strlen((char *)name);
    task->name = block_allocate(name_len);
    task->name_length = name_len;
    memcpy(task->name, name, name_len);

    // allocate a syscall stack
    task->syscall_stack_top = (uint64_t)page_allocate(1) + PAGE;

    task->empty = true; // mark as an empty task (i.e. not tied to an executable)

    spinlock_acquire(&last_id_lock);
    task->id = ++last_id;
    spinlock_release(&last_id_lock);

    // allocate the file descriptor translation
    task->fd_allocated_pages = 1;
    task->fd_count = 1; // stdin, stdout, stderr
    task->fd_translation = page_allocate(1);
    task->fd_max = PAGE / sizeof(vfs_fs_node_t *);

    // find the least busy core
    size_t min = UINT64_MAX;
    scheduler_context_t *free_context = arch_get_scheduler_context(), *current_context = last_context;

    do
    {
        if (current_context->running.count < min)
        {
            min = current_context->running.count;
            free_context = current_context;
        }

        current_context = current_context->previous; // we start with the last context, thus we have to iterate backwards
    } while (current_context);

    task_scheduler_round_robin_push_to_queue(&free_context->running, task); // push it tot the queue

    return task;
}

scheduler_task_t *task_scheduler_round_robin_create_kernel(const char *name, void *task)
{
    scheduler_task_t *new_task = task_scheduler_round_robin_create(name);

#ifdef ARCH_x86_64
    new_task->state.cr3 = (uint64_t)arch_bootstrap_page_table - kernel_hhdm_offset; // cr3 has to be a physical address
    new_task->state.rip = (uint64_t)task;                                           // point to the designated entry point
    new_task->state.rflags = 0x202;                                                 // enable interrupts
    new_task->state.rsp = (uint64_t)page_allocate(1) + PAGE;                        // point to the stack (on x86_64 the stack grows down)

    // point to kernel-space segements
    new_task->state.cs = 8 * 1 | 0;
    new_task->state.ss = 8 * 2 | 0;
#endif

    return new_task;
}

/*

Initialisation/Enabling

*/

noreturn void task_scheduler_round_robin_enable()
{
    scheduler_context_t *context = (scheduler_context_t *)arch_get_scheduler_context();
    scheduler_task_t *task = task_scheduler_round_robin_pop_from_queue(&context->running);

    arch_restore_simd_state(&task->simd_state); // restore simd
    clock_preemption_timer.schedule_one_shot(); // schedule a timer interrupt
    arch_switch_state(&task->state);            // switch to the task
}

void task_scheduler_round_robin_init()
{
    task_scheduler_round_robin_install_context();
}

void task_scheduler_round_robin_yield()
{
    clock_preemption_timer.interrupt_now();
}