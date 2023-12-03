#pragma once
#include <config.h>

#if (SCHEDULER_DESIRED_TASK_SCHEDULER == ROUND_ROBIN)
#include <schedulers/task/roundrobin/roundrobin.h>

#define task_scheduler_init task_scheduler_round_robin_init
#endif