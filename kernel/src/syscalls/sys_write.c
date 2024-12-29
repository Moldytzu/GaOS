#define MODULE "sys_write"
#include <misc/logger.h>

#include <syscalls/helpers.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html

int64_t sys_write(uint64_t num, uint64_t fd, char *buffer, size_t size, size_t offset)
{
    // fixme: this is pwrite

    used(num), used(offset);
    scheduler_task_t *caller = GET_CALLER_TASK();

    // sanitize address
    if (!IS_USER_MEMORY(buffer, caller))
    {
        log_error("failed to write invalid pointer %p from %s", buffer, caller->name);
        return -EPERM;
    }

    // verify fd
    if (fd >= caller->fd_count)
        return -EINVAL;

    vfs_fs_node_t *node = caller->fd_translation[fd];
    vfs_write(node, buffer, size, offset); // todo: use io async and block thread

    return 0;
}