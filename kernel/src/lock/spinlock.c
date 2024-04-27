#include <lock/spinlock.h>
#include <arch/arch.h>

void spinlock_acquire(spinlock_t *lock)
{
    while (1)
    {
        if (__sync_bool_compare_and_swap(lock, 0, 1))
            break;
        arch_hint_spinlock();
    }
}

void spinlock_release(spinlock_t *lock)
{
    __atomic_store_n(lock, 0, __ATOMIC_SEQ_CST);
}