#pragma once
#include <misc/libc.h>
#include <arch/arch.h>

struct scheduler_task
{
    uint64_t id;
    char *name;
    size_t name_length;

    arch_cpu_state_t state;
    arch_simd_state_t simd_state;

    bool empty;

    struct scheduler_task_queue *parent_queue;

    struct scheduler_task *next;
    struct scheduler_task *previous;
};

typedef struct scheduler_task scheduler_task_t;

struct scheduler_task_queue
{
    scheduler_task_t *current;
    scheduler_task_t *head;
    scheduler_task_t *last;
    spinlock_t lock;
    size_t count;
};

typedef struct scheduler_task_queue scheduler_task_queue_t;

struct scheduler_context
{
    uint64_t cpu_id;

    scheduler_task_queue_t running;

    struct scheduler_context *next;
    struct scheduler_context *previous;
};

typedef struct scheduler_context scheduler_context_t;

scheduler_task_t *task_scheduler_round_robin_create(const char *name);
void task_scheduler_round_robin_init(void);
void task_scheduler_round_robin_install_context(void);
noreturn void task_scheduler_round_robin_reschedule(arch_cpu_state_t *state);
noreturn void task_scheduler_round_robin_enable(void);