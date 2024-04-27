#include <devices/timers/timers.h>

#ifdef ARCH_x86_64
#include <devices/timers/acpi_pm/pm.h>
#endif

void timers_init(void)
{
#ifdef ARCH_x86_64
    acpi_pm_timer_init();
#endif
}