#pragma once
#include <misc/libc.h>
#include <syscalls/syscalls.h>
#include <schedulers/task/task.h>

#define GET_CALLER_TASK() ((scheduler_task_t *)arch_get_task_context())
#define GET_PAGE_TABLE(task) ((arch_page_table_t *)(task->state.cr3 + kernel_hhdm_offset))
#define IS_MAPPED(x, page_table) (arch_table_manager_translate_to_physical(page_table, (uint64_t)(x)) != 0)
#define IS_HIGHER_HALF_ADDRESS(x) ((uint64_t)x >= kernel_hhdm_offset)