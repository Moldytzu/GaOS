#pragma once
#include <misc/libc.h>

extern uint64_t syscall_count;
extern void (*syscall_handlers[])();