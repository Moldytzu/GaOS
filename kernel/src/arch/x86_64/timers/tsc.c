#define MODULE "x86_64/tsc"
#include <misc/libc.h>

#include <misc/libc.h>
#include <arch/arch.h>
#include <clock/clock.h>

uint64_t arch_tsc_read_nanoseconds();
void arch_tsc_sleep_nanoseconds(uint64_t nanoseconds);

uint64_t arch_tsc_offset = 0;

static clock_time_source_t arch_tsc_timer = {
    .name = "tsc",
    .time_keeping_capable = true,
    .ticks_per_second = 0,
    .read_nanoseconds = arch_tsc_read_nanoseconds,
    .sleep_nanoseconds = arch_tsc_sleep_nanoseconds,
    .one_shot_capable = false,
};

uint64_t rdtsc()
{
    uint64_t low, high;
    iasm("rdtsc" : "=a"(low), "=d"(high));
    return (high << 32) | low;
}

uint64_t arch_tsc_read_nanoseconds()
{
    return (rdtsc() - arch_tsc_offset) * 1000000000 / arch_tsc_timer.ticks_per_second;
}

void arch_tsc_sleep_nanoseconds(uint64_t nanoseconds)
{
    uint64_t target = arch_tsc_read_nanoseconds() + nanoseconds;
    while (arch_tsc_read_nanoseconds() < target)
        arch_hint_spinlock();
}

void arch_tsc_init()
{
    if (!clock_system_timer.sleep_nanoseconds)
        return;

    // fixme: check cpuid for support

    // perform calibration
    uint64_t a = rdtsc();
    clock_system_timer.sleep_nanoseconds(100 * 1000000 /*nanoseconds to miliseconds*/); // wait 100 ms
    uint64_t b = rdtsc();

    arch_tsc_timer.ticks_per_second = (b - a) * 10; // determine ticks per second

    clock_register_time_source(arch_tsc_timer);
}

void arch_tsc_reset()
{
    // fixme: check cpuid for support
    arch_tsc_offset = rdtsc();
}