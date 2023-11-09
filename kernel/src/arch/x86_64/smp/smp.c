#define MODULE "x86_64/smp"
#include <misc/logger.h>

#include <arch/x86_64/arch.h>
#include <arch/x86_64/xapic/xapic.h>
#include <acpi/acpi.h>
#include <misc/libc.h>
#include <boot/limine.h>

int arch_bootstrap_ap()
{
    return 0;
}