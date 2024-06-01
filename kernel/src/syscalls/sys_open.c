#define MODULE "sys_open"
#include <misc/logger.h>

#include <syscalls/helpers.h>

int64_t sys_open(uint64_t num, char *filename, uint64_t mode)
{
    used(num);

    scheduler_task_t *caller = GET_CALLER_TASK();
    arch_page_table_t *caller_pt = GET_PAGE_TABLE(caller);

    // sanitize address
    if (IS_HIGHER_HALF_ADDRESS(filename) || !IS_MAPPED(filename, caller_pt)) // userspace pointers are lower half and mapped
    {
        log_error("failed to open invalid pointer %p from %s", filename, caller->name);
        return -EPERM;
    }

    // open the node while propagating the errors
    vfs_fs_node_t *node = vfs_open(filename, mode);
    if (is_error(node))
        return error_of(node);

    // push the node on the translation table
    // fixme: check for holes
    size_t fd = caller->fd_count++;

    if (caller->fd_count > caller->fd_max)
    {
        // todo: reallocate
    }

    caller->fd_translation[fd] = node;

    return fd;
}