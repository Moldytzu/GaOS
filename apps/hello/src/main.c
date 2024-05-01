#include <stdint.h>

#define SYS_WRITE 0

int64_t _syscall(uint64_t number, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

int _start()
{
    _syscall(SYS_WRITE, 0, (uint64_t) "Hello, Gallium!", 15, 0, 0);
    while (1)
        ;
    return 0;
}