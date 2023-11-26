#pragma once
#include <misc/libc.h>

pstruct
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
arch_processor_state_t;

pstruct
{
    uint16_t size;
    uint64_t offset;
}
arch_idtr_t;

extern arch_idtr_t arch_global_idtr;
extern void arch_idt_load(arch_idtr_t *);
void arch_interrupts_init();
uint16_t arch_interrupts_reserve_kernel_vector();