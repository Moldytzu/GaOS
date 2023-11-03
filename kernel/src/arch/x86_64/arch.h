#pragma once
#include <misc/libc.h>

ifunc void arch_hint_spinlock()
{
    iasm("pause");
}