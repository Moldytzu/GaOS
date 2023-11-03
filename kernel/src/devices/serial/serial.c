#include <devices/serial/serial.h>

#ifdef ARCH_x86_64
#include <devices/serial/uart16550/uart.h>

void serial_init()
{
    uart16550_init();
}

void serial_send_byte(uint8_t byte)
{
    uart16550_send_byte(byte);
}

void serial_send_string(char *string)
{
    uart16550_send_string(string);
}

#else
#pragma message "This architecture doesn't support serial"

void serial_init()
{
}

void serial_send_byte(uint8_t byte)
{
    (void)byte;
}

void serial_send_string(char *string)
{
    (void)string;
}
#endif