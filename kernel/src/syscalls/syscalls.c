#include <syscalls/syscalls.h>
#define DECLARE_SYSCALL(name) void name()

uint64_t syscall_count = 3;

DECLARE_SYSCALL(sys_write);
DECLARE_SYSCALL(sys_open);
DECLARE_SYSCALL(sys_close);
void (*syscall_handlers[])() = {
    sys_write,
    sys_open,
    sys_close,
};