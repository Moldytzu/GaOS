#include <devices/console.h>
#include <devices/manager.h>

void *console_write(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    used(node);
    char *c = buffer + offset;
    for (size_t i = 0; i < size; i++)
        printk("%c", c[i]);
    return buffer + offset;
}

void *console_read(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    used(node), used(size);
    return buffer + offset;
}

void console_create_device()
{
    device_create_at("/console", reserved, console_read, console_write);
}