#pragma once
#include <misc/libc.h>
#include <devices/manager.h>

#ifndef ARCH_x86_64
#error 16650 UART is not supported on this platform
#endif

void uart16550_init();
void uart16550_send_byte(uint8_t byte);
void uart16550_send_string(char *string);
void uart16550_create_device();