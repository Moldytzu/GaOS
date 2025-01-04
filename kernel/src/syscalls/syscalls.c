#include <syscalls/syscalls.h>
#define DECLARE_SYSCALL(name) void name()

uint64_t syscall_count = 5;

DECLARE_SYSCALL(sys_write);
DECLARE_SYSCALL(sys_open);
DECLARE_SYSCALL(sys_close);
DECLARE_SYSCALL(sys_read);
DECLARE_SYSCALL(sys_fork);

void (*syscall_handlers[])() = {
    sys_write,
    sys_open,
    sys_close,
    sys_read,
    sys_fork,
};