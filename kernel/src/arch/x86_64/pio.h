#include <misc/libc.h>

ifunc void arch_pio_write8(uint16_t port, uint8_t val)
{
    iasm("outb %0, %1" ::"a"(val), "Nd"(port));
}

ifunc uint8_t arch_pio_read8(uint16_t port)
{
    uint8_t val;
    iasm("inb %%dx,%%al"
         : "=a"(val)
         : "d"(port));
    return val;
}

ifunc void arch_pio_write16(uint16_t port, uint16_t val)
{
    iasm("outw %0, %1" ::"a"(val), "Nd"(port));
}

ifunc uint16_t arch_pio_read16(uint16_t port)
{
    uint16_t val;
    iasm("inw %%dx,%%ax"
         : "=a"(val)
         : "d"(port));
    return val;
}

ifunc void arch_pio_write32(uint16_t port, uint32_t val)
{
    iasm("out %0, %1" ::"a"(val), "Nd"(port));
}

ifunc uint32_t arch_pio_read32(uint16_t port)
{
    uint32_t val;
    iasm("in %1, %0"
         : "=a"(val)
         : "Nd"(port));
    return val;
}