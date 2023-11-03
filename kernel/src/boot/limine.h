#pragma once
#include <misc/libc.h> // c library
#include <limine.h>    // bootloader interface

extern volatile struct limine_framebuffer_request kernel_framebuffer_request;

void limine_init();