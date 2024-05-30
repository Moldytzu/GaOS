#define MODULE "sys_close"
#include <misc/logger.h>

#include <syscalls/helpers.h>

int64_t sys_close(uint64_t num, uint64_t fd)
{
    used(num);

    scheduler_task_t *caller = GET_CALLER_TASK();

    // sanitize fd
    if (fd >= caller->fd_count || fd < 3)
        return -EINVAL;

    vfs_fs_node_t *node = caller->fd_translation[fd];
    vfs_close(node);
    caller->fd_count--;

    return 0;
}