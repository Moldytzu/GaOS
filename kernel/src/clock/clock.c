#define MODULE "clock"
#include <misc/logger.h>

#include <clock/clock.h>

clock_time_source_t clock_system_timer;
clock_time_source_t clock_preemption_timer;

void clock_register_time_source(clock_time_source_t time_source)
{
    if (time_source.one_shot_capable) // preemption candidate
    {
        if (time_source.ticks_per_second > clock_preemption_timer.ticks_per_second)
        {
            log_info("switching preemption timer to %s (%d ticks per second)", time_source.name, time_source.ticks_per_second);
            clock_preemption_timer = time_source;
        }
    }

    if (time_source.time_keeping_capable) // system candidate
    {
        if (time_source.ticks_per_second > clock_system_timer.ticks_per_second)
        {
            log_info("switching system timer to %s (%d ticks per second)", time_source.name, time_source.ticks_per_second);
            clock_system_timer = time_source;
        }
    }
}