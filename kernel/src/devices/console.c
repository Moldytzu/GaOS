#include <devices/console.h>
#include <devices/manager.h>

ssize_t console_write(struct vfs_fs_node *node, void *buffer, size_t size)
{
    used(node);
    char *c = buffer;
    for (size_t i = 0; i < size; i++)
        printk("%c", c[i]);
    return size;
}

ssize_t console_read(struct vfs_fs_node *node, void *buffer, size_t size)
{
    used(node), used(size), used(buffer);
    return 0;
}

void console_create_device()
{
    device_create_at("/console", reserved, nullptr, console_read, console_write);
}