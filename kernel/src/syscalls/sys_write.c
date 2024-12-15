#define MODULE "sys_write"
#include <misc/logger.h>

#include <syscalls/helpers.h>

int64_t sys_write(uint64_t num, uint64_t fd, char *buffer, size_t size, size_t file_offset)
{
    used(num), used(fd), used(file_offset);
    scheduler_task_t *caller = GET_CALLER_TASK();

    // todo: use fd

    // sanitize address
    if (!IS_USER_MEMORY(buffer, caller))
    {
        log_error("failed to write invalid pointer %p from %s", buffer, caller->name);
        return -EPERM;
    }

    // write the buffer to kernel log
    for (size_t i = 0; i < size; i++)
    {
        printk("%c", buffer[i]);
        printk_serial("%c", buffer[i]);
    }

    return 0;
}