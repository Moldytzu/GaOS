#pragma once
#include <misc/libc.h>
#include <arch/arch.h>
#include <filesystem/vfs.h>

struct scheduler_task
{
    // cpu state (must have 256 bytes alignment)
    uint64_t id;
    uint64_t syscall_stack_top;
    uint64_t userspace_stack_top;
    arch_simd_state_t simd_state;
    arch_cpu_state_t state;
    uint64_t pad[7];

    // metadata
    char *name;
    size_t name_length;
    bool empty;

    // file descriptors
    size_t fd_count;
    size_t fd_max;
    vfs_fs_node_t **fd_translation;
    size_t fd_allocated_pages;

    // list
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
scheduler_task_t *task_scheduler_round_robin_create_kernel(const char *name, void *task);
void task_scheduler_round_robin_init();
void task_scheduler_round_robin_install_context();
noreturn void task_scheduler_round_robin_reschedule(arch_cpu_state_t *state);
noreturn void task_scheduler_round_robin_enable();
void task_scheduler_round_robin_yield();
scheduler_task_t *task_scheduler_round_robin_get_child(scheduler_task_t *parent, uint64_t pid);