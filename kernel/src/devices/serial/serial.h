#pragma once
#include <misc/libc.h>

void serial_init();
void serial_send_byte(uint8_t byte);
void serial_send_string(char *string);