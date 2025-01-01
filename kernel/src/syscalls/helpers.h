#pragma once
#include <misc/libc.h>
#include <syscalls/syscalls.h>
#include <schedulers/task/task.h>

#define TRACE_INFOS
#define TRACE_ERRORS

#define GET_CALLER_TASK() ((scheduler_task_t *)arch_get_task_context())
#define GET_PAGE_TABLE(task) ((arch_page_table_t *)(task->state.cr3 + kernel_hhdm_offset))
#define IS_MAPPED(x, page_table) (arch_table_manager_translate_to_physical(page_table, (uint64_t)(x)) != 0)
#define IS_HIGHER_HALF_ADDRESS(x) ((uint64_t)x >= kernel_hhdm_offset)
#define IS_USER_MEMORY(address, task) (!IS_HIGHER_HALF_ADDRESS((address)) && IS_MAPPED((address), GET_PAGE_TABLE(task)))

#ifdef TRACE_ERRORS
#define trace_error(fmt, ...) log_error((fmt), ##__VA_ARGS__)
#else
#define trace_error(fmt, ...) ()
#endif

#ifdef TRACE_INFOS
#define trace_info(fmt, ...) log_info((fmt), ##__VA_ARGS__)
#else
#define trace_info(fmt, ...) ()
#endif