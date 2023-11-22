#pragma once
#include <stdint.h>

ifunc void arch_cpuid(uint32_t reg, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    iasm("cpuid"
         : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
         : "0"(reg));
}