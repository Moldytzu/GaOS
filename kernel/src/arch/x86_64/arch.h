#pragma once
#include <misc/libc.h>
#include <arch/x86_64/pio.h>                              // I/O ports
#include <arch/x86_64/simd/simd.h>                        // SSE/AVX
#include <arch/x86_64/smp/smp.h>                          // SMP
#include <arch/x86_64/stack/stack.h>                      // stack
#include <arch/x86_64/page_table_manager/table_manager.h> // page tables
#include <arch/x86_64/lock/spinlock.h>                    // spinlocks
#include <arch/x86_64/idt/idt.h>                          // interrupts
#include <arch/x86_64/xapic/xapic.h>                      // interrupt controller

extern uint64_t arch_trampoline_base;

ifunc void arch_hint_spinlock()
{
    iasm("pause" ::: "memory");
}

ifunc void arch_hint_serialize()
{
    iasm("mfence" ::: "memory");
}

void arch_init();