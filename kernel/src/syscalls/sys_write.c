#define MODULE "sys_write"
#include <misc/logger.h>

#include <syscalls/helpers.h>

int64_t sys_write(uint64_t num, uint64_t fd, char *buffer, size_t size)
{
    used(num), used(fd);
    scheduler_task_t *caller = GET_CALLER_TASK();
    arch_page_table_t *caller_pt = GET_PAGE_TABLE(caller);

    if (IS_HIGHER_HALF_ADDRESS(buffer) || !IS_MAPPED(buffer, caller_pt)) // userspace pointers are lower half and mapped
    {
        log_error("failed to write invalid pointer %p from %s", buffer, caller->name);
        return -1;
    }

    for (size_t i = 0; i < size; i++)
    {
        printk("%c", buffer[i]);
        printk_serial("%c", buffer[i]);
    }

    return 0;
}