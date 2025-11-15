#define MODULE "sys_waitpid"
#include <misc/logger.h>

#include <syscalls/helpers.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/waitpid.html

int64_t sys_waitpid(uint64_t num, uint64_t pid, int *stat_loc, int options)
{
    used(num), used(pid), used(stat_loc), used(options);
    printk_serial_unsafe("waitpid called for pid %d\n", pid);
    if (task_scheduler_get_pid(pid))
    {
        printk_serial_unsafe("waitpid: found pid %d\n", pid);
        return 1;
    }
    printk_serial_unsafe("waitpid: no such pid %d\n", pid);
    return -1;
}