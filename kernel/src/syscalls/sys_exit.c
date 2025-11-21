#define MODULE "sys_exit"
#include <misc/logger.h>

#include <syscalls/helpers.h>
#include <schedulers/task/task.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/exit.html

int64_t sys_exit(uint64_t num, int code)
{
    used(num);
    scheduler_task_t *caller = GET_CALLER_TASK();
    caller->exit_code = code;
    caller->run_mode = RUN_MODE_ZOMBIE;
    trace_info("exit code %d", code);
    while (1)
        task_scheduler_yield();
    return 0;
}