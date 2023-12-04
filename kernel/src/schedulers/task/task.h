#pragma once
#include <config.h>

#if (SCHEDULER_DESIRED_TASK_SCHEDULER == ROUND_ROBIN)
#include <schedulers/task/round_robin/round_robin.h>

#define task_scheduler_init task_scheduler_round_robin_init
#define task_scheduler_install_context task_scheduler_round_robin_install_context

#else
#error no task scheduler selected. check config.h
#endif