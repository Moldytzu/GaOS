#define MODULE "sys_lseek"
#include <misc/logger.h>

#include <syscalls/helpers.h>
#include <schedulers/task/task.h>
#include <filesystem/vfs.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html

int64_t sys_lseek(uint64_t num, uint64_t fd, ssize_t offset, int whence)
{
    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();

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

    return node->fs->lseek(node, offset, whence);
}