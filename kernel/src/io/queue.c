#define MODULE "io/queue"
#include <misc/logger.h>

#include <io/queue.h>
#include <schedulers/task/task.h>
#include <devices/serial/serial.h>
#include <clock/clock.h>
#include <memory/physical/block_allocator.h>

io_task_t *queue_start = NULL;
io_task_t *queue_last = NULL;
spinlock_t queue_lock;

void io_executor_task()
{
    while (1)
    {
        // wait for a reschedule
        arch_interrupts_enable();
        arch_interrupts_wait();
        arch_interrupts_disable();

        // update all tasks
        spinlock_acquire(&queue_lock);
        for (io_task_t *task = queue_start; task; task = task->next)
        {
            if (task->done) // if it's done, don't do anything
                continue;

            vfs_async_task_update(task->node, task);

            // if it's done unregister it
            if (task->done)
                vfs_async_task_unregister(task->node, task);
        }
        spinlock_release(&queue_lock);
    }
}

void io_queue_init()
{
    task_scheduler_round_robin_create_kernel("io executor", io_executor_task);
}

io_task_t *io_queue_task(vfs_fs_node_t *node, io_op_t operation, void *buffer, size_t size, size_t offset)
{
    // create the structure metadata
    io_task_t *new_task = block_allocate(sizeof(io_task_t));
    new_task->done = false;
    new_task->buffer = buffer;
    new_task->offset = offset;
    new_task->size = size;
    new_task->operation = operation;
    new_task->node = node;

    // try to register as async i/o
    new_task->is_async = vfs_async_task_register(node, new_task);

    if (!new_task->is_async)
    {
        // todo: what should we do?
        // maybe use the slow read/write functions?
    }

    // add in the queue
    spinlock_acquire(&queue_lock);

    if (queue_start)
    {
        queue_last->next = new_task;
        new_task->prev = queue_last;
        queue_last = new_task;
    }
    else
        queue_start = queue_last = new_task;

    spinlock_release(&queue_lock);

    return new_task;
}