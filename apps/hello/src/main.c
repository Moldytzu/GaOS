#include <stdint.h>

uint64_t _syscall(uint64_t number, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

int _start()
{
    _syscall(0, 1, 2, 3, 4, 5);
    while (1)
        ;
    return 0;
}