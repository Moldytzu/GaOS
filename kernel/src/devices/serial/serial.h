#pragma once
#include <misc/libc.h>
#include <devices/manager.h>

void serial_init(void);
void serial_send_byte(uint8_t byte);
void serial_send_string(char *string);
void serial_create_device(void);