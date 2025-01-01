#define MODULE "sys_read"
#include <misc/logger.h>

#include <syscalls/helpers.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html

int64_t sys_read(uint64_t num, uint64_t fd, char *buffer, size_t size)
{
    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();

    // verify pointer
    if (!IS_USER_MEMORY(buffer, caller))
    {
        trace_error("failed to read invalid pointer %p from %s", buffer, caller->name);
        return -EPERM;
    }

    // verify fd
    if (fd >= caller->fd_count)
    {
        trace_error("fd %d of %s is invalid", fd, caller->name);
        return -EBADF;
    }

    vfs_fs_node_t *node = caller->fd_translation[fd];

    if (node == nullptr)
    {
        trace_error("fd %d of %s is invalid", fd, caller->name);
        return -EBADF;
    }

    // make sure the size doesn't overflow the fd
    if (node->seek_position + size > node->max_seek_position)
        size = node->max_seek_position - node->seek_position;

    // call the filesystem
    int64_t status = error_of(vfs_read(node, buffer, size, node->seek_position));

    // if the operation is sucessful increase the seek position
    if (status == 0)
        node->seek_position += size;

    return status;
}