#define MODULE "x86_64/idt"
#include <misc/logger.h>
#include <misc/panic.h>

#include <arch/x86_64/idt/idt.h>
#include <arch/arch.h>
#include <memory/physical/page_allocator.h>

pstruct
{
    uint16_t size;
    uint64_t offset;
}
arch_idtr_t;

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

arch_idtr_t arch_global_idtr;     // fixme: move this in a per-cpu strucutre, while keeping it the same across them
extern void *arch_isr_handlers[]; // array of handlers

uint64_t arch_read_cr2()
{
    uint64_t value = 0;
    iasm("mov %%cr2, %0" ::"r"(value));
    return value;
}

void arch_isr_handler(arch_processor_state_t *state, uint64_t interrupt_number)
{
    if (interrupt_number == 0x2) // NMI
        halt();

    if (interrupt_number < 0x20)
    {
        // handle exceptions
        if (interrupt_number == 0xE) // page fault
        {
            uint32_t error = state->error;
            panic("Page Fault caused by %s page, on a %s in %s at 0x%x", error & 1 ? "present" : "non-present", error & 0b10 ? "write" : "read", error & 0b100 ? "user-mode" : "kernel-mode", arch_read_cr2());
        }
        else
            panic("exception 0x%x", interrupt_number);
    }
    else
    {
        // handle interrupt
        log_info("interrupt 0x%x", interrupt_number);
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

extern void arch_idt_load(arch_idtr_t *);
void arch_interrupts_init()
{
    arch_global_idtr.offset = (uint64_t)page_allocate(1);
    arch_global_idtr.size = 256 * sizeof(arch_idt_gate_descriptor_t) - 1; // there are 256 vectors, thus the size will be this

    for (size_t i = 0; i < 256; i++)
        arch_interrupts_map_vector(i, arch_isr_handlers[i]);

    arch_idt_load(&arch_global_idtr);
}