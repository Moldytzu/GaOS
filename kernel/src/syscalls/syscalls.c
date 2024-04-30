#include <syscalls/syscalls.h>
#define DECLARE_SYSCALL(name) void name(void)

uint64_t syscall_count = 1;

DECLARE_SYSCALL(sys_write);
void (*syscall_handlers[])(void) = {sys_write};