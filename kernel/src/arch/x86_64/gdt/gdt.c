#define MODULE "x86_64/gdt"
#include <misc/logger.h>

#include <arch/x86_64/gdt/gdt.h>
#include <misc/libc.h>
#include <memory/physical/page_allocator.h>
#include <boot/limine.h>

pstruct
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    unsigned limit2 : 4;
    unsigned flags : 4;
    uint8_t base3;
}
gdt_segment_t;

pstruct
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    uint8_t limit2;
    uint8_t base3;
    uint32_t base4;
    uint32_t ignore;
}
gdt_system_segment_t;

pstruct
{
    uint32_t ignore;
    uint64_t rsp[3]; // stack pointers
    uint64_t ignore2;
    uint64_t ist[7]; // intrerrupt stack table
    uint64_t ignore3;
    uint16_t ignore4;
    uint16_t iopb; // io map base address
}
gdt_tss_t;

pstruct
{
    uint16_t size;
    gdt_segment_t *entries;
}
gdtr_t;

pstruct
{
    gdtr_t gdtr;
    gdt_segment_t null align_addr(16);
    gdt_segment_t kernel_data;
    gdt_segment_t kernel_code;
    gdt_segment_t user_data;
    gdt_segment_t user_code;
    gdt_system_segment_t tss_segment;
    gdt_tss_t tss align_addr(16);
}
gdt_info_t;

extern void arch_gdt_load(gdtr_t *);

void arch_load_gdt()
{
    // todo: remember this in a per-cpu structure
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

    arch_gdt_load(&info->gdtr);
}