
#include <misc/libc.h>
#include <syscalls/syscalls.h>

uint64_t sys_write(uint64_t num, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5)
{
    printk_serial("%p: %p %p %p %p %p\n", num, param1, param2, param3, param4, param5);
    return 1;
}