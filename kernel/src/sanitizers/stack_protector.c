#include <misc/libc.h>
#include <misc/panic.h>

uint64_t __stack_chk_guard = 0xDEADBEEFF00DCACA;

noreturn void __stack_chk_fail()
{
    panic("Stack smashing detected.");
}