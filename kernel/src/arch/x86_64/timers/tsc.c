#define MODULE "x86_64/tsc"
#include <misc/logger.h>

#include <misc/libc.h>
#include <arch/arch.h>
#include <clock/clock.h>

uint64_t arch_tsc_read_nanoseconds(void);
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

uint64_t rdtsc(void)
{
    uint64_t low, high;
    iasm("rdtsc" : "=a"(low), "=d"(high));
    return (high << 32) | low;
}

uint64_t arch_tsc_read_nanoseconds(void)
{
    return (rdtsc() - arch_tsc_offset) / (arch_tsc_timer.ticks_per_second / 1000000000ULL /*seconds to nanoseconds*/);
}

void arch_tsc_sleep_nanoseconds(uint64_t nanoseconds)
{
    uint64_t target = arch_tsc_read_nanoseconds() + nanoseconds;
    while (arch_tsc_read_nanoseconds() < target)
        arch_hint_spinlock();
}

void arch_tsc_init(void)
{
    if (!clock_system_timer.sleep_nanoseconds)
        return;

    // detect family and model
    uint32_t eax, ebx, ecx, edx;
    uint32_t model, family;
    arch_cpuid(0x00000001, &eax, &ebx, &ecx, &edx);

    family = (eax >> 8) & 0xF;
    model = (eax >> 4) & 0xF;
    if (family >= 6)
        model = ((eax >> 16) << 4);
    if (family == 15)
        family += (eax >> 20);

    // todo: check these in another part of code

    if (family >= 7 || (family == 6 && model >= 0xF)) // new enough to have a constant tsc
    {
        // perform calibration
        uint64_t a = rdtsc();
        clock_system_timer.sleep_nanoseconds(100 * 1000000 /*nanoseconds to miliseconds*/); // wait 100 ms
        uint64_t b = rdtsc();

        arch_tsc_timer.ticks_per_second = (b - a) * 10; // determine ticks per second

        clock_register_time_source(arch_tsc_timer);
    }
    else
        log_warn("unreliable tsc detected");
}

void arch_tsc_reset(void)
{
    // fixme: do this on each cpu independently
    arch_tsc_offset = rdtsc();
}