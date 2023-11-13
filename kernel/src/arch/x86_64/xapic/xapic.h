#pragma once
#define MSR_APIC_BASE 0x1B

#define XAPIC_REG_ID 0x20
#define XAPIC_REG_SIV 0xF0
#define XAPIC_REG_EOI 0xB0
#define XAPIC_REG_TPR 0x80
#define XAPIC_REG_DFR 0xE0
#define XAPIC_REG_LDR 0xD0
#define XAPIC_REG_ICR_LOW 0x300
#define XAPIC_REG_ICR_HIGH 0x310
#define XAPIC_REG_TIMER_DIV 0x3E0
#define XAPIC_REG_TIMER_INITCNT 0x380
#define XAPIC_REG_TIMER_CURRENTCNT 0x390
#define XAPIC_REG_LVT_TIMER 0x320
#define XAPIC_REG_LVT0 0x350
#define XAPIC_REG_LVT1 0x360

#define XAPIC_TIMER_VECTOR 0x20
#define XAPIC_NMI_VECTOR 0x02

#define XAPIC_BASE (0xFEE00000)

uint64_t arch_get_id();
void arch_xapic_init(bool bsp);
void arch_kill_ap();

ifunc void xapic_write(uint64_t offset, uint32_t value)
{
    *((volatile uint32_t *)((uint8_t *)XAPIC_BASE + offset)) = value;
}

ifunc uint32_t xapic_read(uint64_t offset)
{
    return *((volatile uint32_t *)((uint8_t *)XAPIC_BASE + offset));
}