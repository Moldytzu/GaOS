#pragma once
#include <stdint.h>

typedef uint64_t arch_spinlock_t;

void arch_spinlock_acquire(arch_spinlock_t *);
void arch_spinlock_release(arch_spinlock_t *);