#define MODULE "sys_read"
#include <misc/logger.h>

#include <syscalls/helpers.h>

int64_t sys_read(uint64_t num, uint64_t fd, char *buffer, size_t size, size_t file_offset)
{
    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();
    arch_page_table_t *caller_pt = GET_PAGE_TABLE(caller);

    // todo: use fd

    // sanitize address
    if (IS_HIGHER_HALF_ADDRESS(buffer) || !IS_MAPPED(buffer, caller_pt)) // userspace pointers are lower half and mapped
    {
        log_error("failed to write invalid pointer %p from %s", buffer, caller->name);
        return -EPERM;
    }

    // sanitize fd
    if (fd >= caller->fd_count || fd < 3)
        return -EINVAL;

    vfs_fs_node_t *node = caller->fd_translation[fd];
    vfs_read(node, buffer, size, file_offset); // todo: use io async and block thread

    return 0;
}