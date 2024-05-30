#include <syscalls/syscalls.h>
#define DECLARE_SYSCALL(name) void name()

uint64_t syscall_count = 2;

DECLARE_SYSCALL(sys_write);
DECLARE_SYSCALL(sys_open);
void (*syscall_handlers[])() = {
    sys_write,
    sys_open,
};