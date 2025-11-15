#pragma once
#include <misc/libc.h>

typedef struct
{
    const char *name;

    bool time_keeping_capable; // if possible to read nanoseconds
    uint64_t ticks_per_second; // higher is better in terms of resolution
    uint64_t (*read_nanoseconds)();
    void (*sleep_nanoseconds)(uint64_t);

    bool one_shot_capable;
    void (*schedule_one_shot)(); // this will program a one shot interrupt that will make preemption possible
    void (*interrupt_now)();     // this will simulate an interrupt from this timer
} clock_time_source_t;

extern clock_time_source_t clock_system_timer;
extern clock_time_source_t clock_preemption_timer;

void clock_register_time_source(clock_time_source_t time_source);