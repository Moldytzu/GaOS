#define MODULE "sys_open"
#include <misc/logger.h>
#include <misc/panic.h>

#include <syscalls/helpers.h>
#include <memory/physical/page_allocator.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html

int64_t sys_open(uint64_t num, char *path, uint64_t oflag)
{
    used(num);

    scheduler_task_t *caller = GET_CALLER_TASK();

    // verify pointer
    if (!IS_USER_MEMORY(path, caller))
    {
        log_error("failed to open invalid pointer %p from %s", path, caller->name);
        return -EPERM;
    }

    // open the node while propagating the errors
    vfs_fs_node_t *node = vfs_open(path, oflag);
    if (is_error(node))
        return error_of(node);

    // push the node on the translation table
try_again:
    size_t fd = 0;
    while (caller->fd_translation[fd] != nullptr && fd <= caller->fd_max)
        fd++;

    if (fd == caller->fd_max)
    {
        size_t old_pages = caller->fd_allocated_pages;
        size_t new_pages = ++caller->fd_allocated_pages;
        caller->fd_translation = page_reallocate(caller->fd_translation, old_pages, new_pages);
        caller->fd_allocated_pages++;
        caller->fd_max += PAGE / sizeof(vfs_fs_node_t *);
        goto try_again;
    }

    caller->fd_translation[fd] = node;
    caller->fd_count++;

    return fd;
}