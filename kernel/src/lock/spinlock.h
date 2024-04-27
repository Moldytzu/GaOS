#pragma once

typedef unsigned long long spinlock_t;

void spinlock_acquire(spinlock_t *);
void spinlock_release(spinlock_t *);

inline static __attribute__((always_inline)) void spinlock_wait_for(spinlock_t *spinlock)
{
    while (*spinlock)
        asm volatile("pause" ::: "memory");
}