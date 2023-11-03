#pragma once
#include <misc/libc.h>
#include <arch/x86_64/pio.h> // I/O ports

ifunc void arch_hint_spinlock()
{
    iasm("pause");
}