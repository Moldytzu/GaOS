#define MODULE "sys_write"
#include <misc/logger.h>

#include <syscalls/helpers.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html

int64_t sys_write(uint64_t num, uint64_t fd, char *buffer, size_t size)
{
    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();

    // sanitize address
    if (!IS_USER_MEMORY(buffer, caller))
    {
        trace_error("failed to write invalid pointer %p from %s", buffer, caller->name);
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

    // call the filesystem
    int64_t status = error_of(vfs_write(node, buffer, size, node->seek_position));

    // if the operation is sucessful increase the seek position
    if (status == 0)
        node->seek_position += size;

    return status;
}