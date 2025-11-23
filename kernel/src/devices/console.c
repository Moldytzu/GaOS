#include <devices/console.h>
#include <devices/manager.h>

ssize_t console_write(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    used(node), used(offset);
    char *c = buffer;
    for (size_t i = 0; i < size; i++)
        printk("%c", c[i]);
    return size;
}

ssize_t console_read(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset)
{
    used(node), used(size), used(buffer), used(offset);
    return 0;
}

void console_create_device()
{
    device_create_at("/console", reserved, console_read, console_write);
}