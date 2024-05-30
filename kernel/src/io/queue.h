#pragma once
#include <misc/libc.h>

typedef enum
{
    read,
    write,
} io_op_t;

struct io_task
{
    // metadata
    bool done;
    bool is_async;
    struct io_task *next;
    struct io_task *prev;

    // operation
    struct vfs_fs_node *node;
    io_op_t operation;
    void *buffer;
    size_t size;
    size_t offset;

    // private metadata
    void *fs_status;
};

typedef struct io_task io_task_t;

void io_queue_init();
io_task_t *io_queue_task(struct vfs_fs_node *node, io_op_t operation, void *buffer, size_t size, size_t offset);