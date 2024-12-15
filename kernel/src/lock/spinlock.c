#include <lock/spinlock.h>
#include <arch/arch.h>
#include <stdatomic.h>

void spinlock_acquire(spinlock_t *lock)
{
    int count = 0;
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire))
    {
        arch_hint_spinlock();

        count++;
        if (count >= 100000000)
        {
            printk_serial_unsafe("deadlock to return to %p with interrupts %s\n", STACK_TRACE_WALK(0), (arch_interrupts_enabled() ? "enabled" : "disabled"));
            count = 0;
        }
    }
}

void spinlock_release(spinlock_t *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}