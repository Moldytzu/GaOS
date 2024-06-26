#pragma once
#include <misc/libc.h>

pstruct align_addr(16)
{
    uint64_t cr3;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t error;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
}
arch_cpu_state_t;

typedef struct align_addr(16)
{
    uint8_t data[512];
} arch_simd_state_t;

extern noreturn void arch_switch_state(arch_cpu_state_t *);
extern void arch_save_simd_state(arch_simd_state_t *);
extern void arch_restore_simd_state(arch_simd_state_t *);