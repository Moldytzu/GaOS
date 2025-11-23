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
        trace_error("failed to read invalid pointer %p", buffer);
        return -EPERM;
    }

    // verify fd
    if (fd >= caller->fd_count)
    {
        trace_error("fd %d is invalid", fd);
        return -EBADF;
    }

    vfs_fs_node_t *node = caller->fd_translation[fd];

    if (node == nullptr)
    {
        trace_error("fd %d is invalid", fd);
        return -EBADF;
    }

    // make sure the size doesn't overflow the fd
    if (node->seek_position + size > node->max_seek_position)
        size = node->max_seek_position - node->seek_position;

    // call the filesystem
    ssize_t read = vfs_read(node, buffer, size, node->seek_position);

    // if the operation is sucessful increase the seek position
    if (read > 0)
        node->seek_position += size;

    trace_info("read %d bytes from fd %d", read, fd);
    return read;
}