#pragma once
#include <misc/libc.h>

typedef uint64_t arch_spinlock_t;

void arch_spinlock_acquire(arch_spinlock_t *);
void arch_spinlock_release(arch_spinlock_t *);

ifunc void arch_spinlock_wait_for(arch_spinlock_t *spinlock)
{
    while (*spinlock)
        iasm("pause" ::: "memory");
}