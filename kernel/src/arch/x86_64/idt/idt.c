#define MODULE "x86_64/idt"
#include <misc/logger.h>
#include <misc/panic.h>

#include <arch/x86_64/idt/idt.h>
#include <arch/arch.h>
#include <memory/physical/page_allocator.h>

pstruct
{
    uint16_t offset;
    uint16_t segment_selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
}
arch_idt_gate_descriptor_t;

uint16_t interrupts_vector_base = 0x20;
arch_idtr_t arch_global_idtr;
extern void *arch_isr_handlers[]; // array of handlers

static uint64_t __attribute__((noinline)) arch_read_cr2(void)
{
    uint64_t value = 0;
    iasm("mov %%cr2, %0" ::"r"(value));
    return value;
}

void arch_isr_handler(arch_cpu_state_t *state, uint64_t interrupt_number)
{
    if (interrupt_number == 0x2 || interrupt_number == 0x8) // NMI, Double Fault
        halt();

    if (interrupt_number < 0x20)
    {
        // handle exceptions
        if (interrupt_number == 0xE) // page fault
        {
            uint32_t error = state->error;
            uint64_t cr2 = arch_read_cr2();
            if (cr2 < 0x1000)
                panic("Null dereference at %x", state->rip);
            else
                panic("Page Fault caused by %s page, on a %s in %s of %x at %x", error & 1 ? "present" : "non-present", error & 0b10 ? "write" : "read", error & 0b100 ? "user-mode" : "kernel-mode", cr2, state->rip);
        }
        else
            panic("Exception %x at %x", interrupt_number, state->rip);
    }
    else
    {
        // handle interrupt
        log_info("Interrupt %x at %p in %s", interrupt_number, state->rip, state->cs == 0x10 ? "kernel-mode" : "user-mode");
    }
}

void arch_interrupts_map_vector(uint64_t vector, void *handler)
{
    arch_idt_gate_descriptor_t *gate = &((arch_idt_gate_descriptor_t *)arch_global_idtr.offset)[vector];

    // point offset to the handler
    gate->offset = (uint64_t)handler;
    gate->offset2 = (uint64_t)handler >> 16;
    gate->offset3 = (uint64_t)handler >> 32;

    gate->segment_selector = 8 * 2; // kernel code
    gate->ist = 1;                  // set the ist
    gate->attributes = 0xEE;        // set gate type to 64-bit interrupt, dpl to 3 and present bit
}

void arch_interrupts_init(void)
{
    arch_global_idtr.offset = (uint64_t)page_allocate(1);
    arch_global_idtr.size = 0xFF * sizeof(arch_idt_gate_descriptor_t) - 1;

    for (size_t i = 0; i < 0xFF; i++)
        arch_interrupts_map_vector(i, arch_isr_handlers[i]);

    arch_idt_load(&arch_global_idtr);

    arch_interrupts_enable();
}

uint16_t arch_interrupts_reserve_kernel_vector(void)
{
    return interrupts_vector_base++;
}