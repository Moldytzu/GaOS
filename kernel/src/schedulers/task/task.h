#pragma once
#include <config.h>

#if (SCHEDULER_DESIRED_TASK_SCHEDULER == ROUND_ROBIN)
#include <schedulers/task/round_robin/round_robin.h>

#define task_scheduler_init task_scheduler_round_robin_init
#define task_scheduler_install_context task_scheduler_round_robin_install_context
#define task_scheduler_reschedule task_scheduler_round_robin_reschedule
#define task_scheduler_enable task_scheduler_round_robin_enable
#define task_scheduler_create task_scheduler_round_robin_create
#define task_scheduler_create_kernel task_scheduler_round_robin_create_kernel

#else
#error no task scheduler selected. check config.h
#endif