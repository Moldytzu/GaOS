#define MODULE "sys_yield"
#include <misc/logger.h>

#include <syscalls/helpers.h>
#include <schedulers/task/task.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html

int64_t sys_yield(uint64_t num)
{
    used(num);
    task_scheduler_yield();
    return 0;
}