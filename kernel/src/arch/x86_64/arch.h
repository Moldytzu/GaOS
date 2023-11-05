#pragma once
#include <misc/libc.h>
#include <arch/x86_64/pio.h>         // I/O ports
#include <arch/x86_64/simd/simd.h>   // SSE/AVX
#include <arch/x86_64/smp/smp.h>     // SMP
#include <arch/x86_64/stack/stack.h> // stack

ifunc void arch_hint_spinlock()
{
    iasm("pause");
}