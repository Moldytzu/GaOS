#pragma once
#include <misc/libc.h>

#define MSR_APIC_BASE 0x1B

#define XAPIC_REG_ID 0x20
#define XAPIC_REG_SIV 0xF0
#define XAPIC_REG_EOI 0xB0
#define XAPIC_REG_TPR 0x80
#define XAPIC_REG_DFR 0xE0
#define XAPIC_REG_LDR 0xD0
#define XAPIC_REG_ICR_LOW 0x300
#define XAPIC_REG_ICR_HIGH 0x310
#define XAPIC_REG_TIMER_DIVISOR 0x3E0
#define XAPIC_REG_TIMER_INIT_COUNT 0x380
#define XAPIC_REG_TIMER_CURRENT_COUNT 0x390
#define XAPIC_REG_LVT_TIMER 0x320
#define XAPIC_REG_LVT0 0x350
#define XAPIC_REG_LVT1 0x360

#define XAPIC_BASE (0xFEE00000)

uint64_t arch_get_id();
void arch_xapic_init(bool bsp);
void arch_kill_ap();
void arch_xapic_write(uint64_t offset, uint32_t value);
uint32_t arch_xapic_read(uint64_t offset);
ifunc void arch_xapic_eoi()
{
    arch_xapic_write(XAPIC_REG_EOI, 0);
}