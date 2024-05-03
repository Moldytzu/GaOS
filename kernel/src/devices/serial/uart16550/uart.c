#define MODULE "uart16550"
#include <misc/logger.h>

#include <devices/serial/uart16550/uart.h>
#include <arch/arch.h>

#define COM1_BASE 0x3F8
#define COM_PORT_BASE COM1_BASE
#define COM_PORT_DATA (COM_PORT_BASE + 0)
#define COM_PORT_INTERRUPT_ENABLE (COM_PORT_BASE + 1)
#define COM_PORT_FIFO_CONTROL (COM_PORT_BASE + 2)
#define COM_PORT_LINE_CONTROL (COM_PORT_BASE + 3)
#define COM_PORT_MODEM_CONTROL (COM_PORT_BASE + 4)
#define COM_PORT_LINE_STATUS (COM_PORT_BASE + 5)
#define COM_PORT_MODEM_STATUS (COM_PORT_BASE + 6)
#define COM_PORT_SCRATCH_REGISTER (COM_PORT_BASE + 7)

void uart16550_init(void)
{
    // set baud rate divisor to 1 (115200 baud)
    uint16_t divisor = 1;
    arch_pio_write8(COM_PORT_LINE_CONTROL, arch_pio_read8(COM_PORT_LINE_CONTROL) | 0b10000000);  // enable divisor registers access
    arch_pio_write8(COM_PORT_BASE + 0, divisor & 0xFF);                                          // send divisor
    arch_pio_write8(COM_PORT_BASE + 1, divisor >> 8);                                            //    in two writes
    arch_pio_write8(COM_PORT_LINE_CONTROL, arch_pio_read8(COM_PORT_LINE_CONTROL) & ~0b10000000); // disable divisor registers access

    // set data bits count to 8
    arch_pio_write8(COM_PORT_LINE_CONTROL, arch_pio_read8(COM_PORT_LINE_CONTROL) & ~0b111); // clear first three bits
    arch_pio_write8(COM_PORT_LINE_CONTROL, arch_pio_read8(COM_PORT_LINE_CONTROL) | 0b11);   // set 8 data bits

    // set stop bits count to 1
    arch_pio_write8(COM_PORT_LINE_CONTROL, arch_pio_read8(COM_PORT_LINE_CONTROL) | 0b000); // set 1 stop bit

    // enable fifo
    arch_pio_write8(COM_PORT_FIFO_CONTROL, 0b110001); // set thresold to 14 bytes

    log_info("initialised");
}

static bool uart16550_can_send(void)
{
    return arch_pio_read8(COM_PORT_LINE_STATUS) & 0b100000; // check if transmission buffer is empty
}

void uart16550_send_byte(uint8_t byte)
{
    if (byte == '\n')              // GaOS uses LF encoding, while most serial terminals use CRLF
        uart16550_send_byte('\r'); // we want to emulate this behaviour

    while (!uart16550_can_send())
        arch_hint_spinlock();

    arch_pio_write8(COM_PORT_DATA, byte);
}

void uart16550_send_string(char *string)
{
    while (*string)
        uart16550_send_byte(*string++);
}

void *uart16550_read(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    (void)node, (void)buffer, (void)size, (void)offset;
    return NULL;
}

void *uart16550_write(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    (void)node;
    char *c = buffer + offset;
    for (size_t i = 0; i <= size; i++)
        uart16550_send_byte(c[i]);
    return buffer + offset;
}

void uart16550_create_device(void)
{
    device_create_at("/uart0", serial, uart16550_read, uart16550_write);
}