#define MODULE "x86_64/gdt"
#include <misc/logger.h>

#include <arch/x86_64/gdt/gdt.h>
#include <misc/libc.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

extern void arch_gdt_load(gdtr_t *);

void arch_load_gdt()
{
    gdt_info_t *info = page_allocate(1);

    info->gdtr.size = sizeof(gdt_segment_t) * 5 + sizeof(gdt_system_segment_t) - 1; // set the size
    info->gdtr.entries = &info->null;

    // set up segments
    info->kernel_data.access = 0b10010010;
    info->kernel_data.flags = 0b1010; // 4k pages, long mode

    info->kernel_code.access = 0b10011010;
    info->kernel_code.flags = 0b1010; // 4k pages, long mode

    info->user_data.access = 0b11110010;
    info->user_data.flags = 0b1010; // 4k pages, long mode

    info->user_code.access = 0b11111010;
    info->user_code.flags = 0b1010; // 4k pages, long mode

    // set up tss
    uint64_t tss_address = (uint64_t)&info->tss;
    info->tss_segment.access = 0b10001001;
    info->tss_segment.limit = sizeof(gdt_tss_t);
    info->tss_segment.base = (uint16_t)tss_address;
    info->tss_segment.base2 = (uint8_t)(tss_address >> 16);
    info->tss_segment.base3 = (uint8_t)(tss_address >> 24);
    info->tss_segment.base4 = (uint32_t)(tss_address >> 32);

    // allocate stacks
    info->tss.ist[0] = (uint64_t)page_allocate(1) + PAGE;
    info->tss.rsp[0] = (uint64_t)page_allocate(1) + PAGE;
    info->tss.rsp[2] = (uint64_t)page_allocate(1) + PAGE;

    arch_gdt_load(&info->gdtr);

    // install a new context
    arch_cpu_context_t *context = arch_context_install();
    context->gdt = info;
}