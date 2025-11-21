#define MODULE "sys_waitpid"
#include <misc/logger.h>

#include <syscalls/helpers.h>
#include <schedulers/task/task.h>

// https://pubs.opengroup.org/onlinepubs/9799919799/functions/waitpid.html

int64_t sys_waitpid(uint64_t num, int64_t pid, int *stat_loc, int options)
{
    used(num), used(pid), used(stat_loc), used(options);
    scheduler_task_t *refered_task = task_scheduler_get_pid(pid);
    scheduler_task_t *caller = GET_CALLER_TASK();

    if (pid < 0 /*wait is unimplemented*/ || !refered_task || refered_task->is_waited)
    {
        trace_error("pid %d is invalid", pid);
        return -ECHILD; // pid does not exist
    }

    if (!IS_USER_MEMORY(stat_loc, caller))
    {
        trace_error("invalid pointer %p", stat_loc);
        return -EFAULT; // stat_loc is an invalid pointer
    }

    // check if the refered task is a child of the current task
    scheduler_task_t *current_task = refered_task;
    bool is_child = false;
    do
    {
        if (current_task->ppid == caller->id)
        {
            is_child = true;
            break;
        }
        current_task = task_scheduler_get_pid(current_task->ppid);
    } while (!is_child && current_task);

    if (!is_child || refered_task->is_waited)
        return -ECHILD; // pid is not a child of the caller

    refered_task->is_waited = true;

    trace_info("waiting for pid %d", pid);

    // wait for the task to exit
    while (refered_task->run_mode != RUN_MODE_ZOMBIE)
        task_scheduler_yield();

    trace_info("pid %d exited with code %d", pid, refered_task->exit_code);

    // fill stat_loc
    int to_return = 0;
    to_return |= refered_task->exit_code & 0xFF; // exit code in the lower byte
    *stat_loc = to_return;

    // todo: clean up task

    return pid;
}