#define MODULE "sys_read"
#include <misc/logger.h>

#include <syscalls/helpers.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html

int64_t sys_read(uint64_t num, uint64_t fd, char *buffer, size_t size, size_t file_offset)
{
    // fixme: this is pread

    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();

    // verify pointer
    if (!IS_USER_MEMORY(buffer, caller))
    {
        log_error("failed to read invalid pointer %p from %s", buffer, caller->name);
        return -EPERM;
    }

    // verify fd
    if (fd >= caller->fd_count)
        return -EBADF;

    vfs_fs_node_t *node = caller->fd_translation[fd];
    return error_of(vfs_read(node, buffer, size, file_offset));
}