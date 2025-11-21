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
#define trace_error(fmt, ...) printk_serial_unsafe("[%s (pid %d) error %s @ %d.%d%d%d] " fmt "\n",                                                               \
                                                   GET_CALLER_TASK()->name,                                                                                      \
                                                   GET_CALLER_TASK()->id,                                                                                        \
                                                   MODULE,                                                                                                       \
                                                   (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 / 1000 : 0),       \
                                                   (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 1000 / 100 : 0), \
                                                   (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 100 / 10 : 0),   \
                                                   (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 10 : 0),         \
                                                   ##__VA_ARGS__)
#else
#define trace_error(fmt, ...) ()
#endif

#ifdef TRACE_INFOS
#define trace_info(fmt, ...) printk_serial_unsafe("[%s (pid %d) info %s @ %d.%d%d%d] " fmt "\n",                                                                \
                                                  GET_CALLER_TASK()->name,                                                                                      \
                                                  GET_CALLER_TASK()->id,                                                                                        \
                                                  MODULE,                                                                                                       \
                                                  (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 / 1000 : 0),       \
                                                  (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 1000 / 100 : 0), \
                                                  (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 100 / 10 : 0),   \
                                                  (clock_system_timer.time_keeping_capable ? clock_system_timer.read_nanoseconds() / 1000000 % 10 : 0),         \
                                                  ##__VA_ARGS__)
#else
#define trace_info(fmt, ...) ()
#endif