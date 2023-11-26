#define MODULE "x86_64/xapic"
#include <misc/logger.h>

#include <arch/arch.h>
#include <arch/x86_64/timers/xapic.h>
#include <arch/x86_64/xapic/xapic.h>
#include <clock/clock.h>
#include <misc/panic.h>

// todo: create a proper kernel configuration system
#define DESIRED_SCHEDULING_FREQUENCY 100

uint32_t ticks_per_scheduling_burst;
uint16_t xapic_vector;

void arch_xapic_timer_schedule_one_shot()
{
    arch_xapic_eoi();
    arch_xapic_write(XAPIC_REG_TIMER_INIT_COUNT, ticks_per_scheduling_burst); // set count to the calibrated ticks
}

static clock_time_source_t arch_xapic_timer = {
    .name = "xapic",
    .time_keeping_capable = false,
    .ticks_per_second = 0,
    .one_shot_capable = true,
    .schedule_one_shot = arch_xapic_timer_schedule_one_shot,
};

void arch_xapic_timer_init()
{
    if (!clock_system_timer.sleep_nanoseconds)
        panic("no system timer available for APIC timer calibration");

    // 11.5.4 Volume 3 Intel SDM

    xapic_vector = arch_interrupts_reserve_kernel_vector();

    arch_interrupts_disable();

    // calibrate
    arch_xapic_write(XAPIC_REG_TIMER_DIVISOR, 0b0011);        // divide by 16
    arch_xapic_write(XAPIC_REG_LVT_TIMER, xapic_vector);      // oneshot mode
    arch_xapic_write(XAPIC_REG_TIMER_INIT_COUNT, 0xFFFFFFFF); // initialise with max, making it to overflow

    clock_system_timer.sleep_nanoseconds(1000000000 /*one sec in nanos*/ / DESIRED_SCHEDULING_FREQUENCY);

    ticks_per_scheduling_burst = 0xFFFFFFFF - arch_xapic_read(XAPIC_REG_TIMER_CURRENT_COUNT);
    arch_xapic_timer.ticks_per_second = ticks_per_scheduling_burst * 1000 / DESIRED_SCHEDULING_FREQUENCY;

    arch_xapic_write(XAPIC_REG_TIMER_INIT_COUNT, 0); // reset

    arch_interrupts_enable();

    arch_xapic_timer_schedule_one_shot();

    clock_register_time_source(arch_xapic_timer);
}