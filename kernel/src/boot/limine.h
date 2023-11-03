#pragma once
#include <misc/libc.h> // c library
#include <limine.h>    // bootloader interface

extern volatile struct limine_framebuffer_request kernel_framebuffer_request;
extern volatile struct limine_rsdp_request kernel_rsdp_request;

extern uint64_t kernel_hhdm_offset;

void limine_init();