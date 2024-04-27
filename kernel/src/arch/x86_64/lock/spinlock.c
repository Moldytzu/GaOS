#include <arch/arch.h>

// fixme: this is portable, move out of arch

void arch_spinlock_acquire(arch_spinlock_t *lock)
{
    while (1)
    {
        if (__sync_bool_compare_and_swap(lock, 0, 1))
            break;
        arch_hint_spinlock();
    }
}

void arch_spinlock_release(arch_spinlock_t *lock)
{
    __atomic_store_n(lock, 0, __ATOMIC_SEQ_CST);
}