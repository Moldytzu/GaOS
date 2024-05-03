#define MODULE "device_manager"
#include <misc/logger.h>

#include <devices/manager.h>
#include <devices/serial/serial.h>
#include <devices/timers/timers.h>
#include <memory/physical/block_allocator.h>

typedef struct
{
    vfs_fs_node_t vfs_header;
    device_t *device;
} device_node_t;

vfs_fs_ops_t devfs;
device_t *device_list = NULL;

void list_devices(void)
{
    char *type_string[] = {"other", "serial", "framebuffer", "timer", "hmi"};
    for (device_t *dev = device_list; dev; dev = dev->next)
    {
        printk_serial("device: %s is %s\n", dev->name, type_string[dev->type]);
    }
}

device_t *device_create_at(char *path, device_type_t type, void *read, void *write)
{
    log_info("adding device %s", path);

    // todo: parse the hierarchy in path
    device_t *dev = block_allocate(sizeof(device_t));
    size_t path_len = strlen(path);
    dev->name = block_allocate(path_len);
    dev->name_length = path_len;
    dev->read = read;
    dev->write = write;
    dev->type = type;
    memcpy(dev->name, path, path_len);

    // add it in the list
    if (device_list)
    {
        device_t *d = device_list;
        while (d->next)
            d = d->next;
        d->next = dev;
    }
    else
        device_list = dev;

    list_devices();

    return dev;
}

char *device_get_by_type(device_type_t type, char *path, size_t path_len, uint64_t index)
{
    uint64_t c = 0;

    for (device_t *dev = device_list; dev; dev = dev->next)
    {
        if (dev->type == type && c++ == index)
        {
            memcpy(path, dev->name, min(strlen(dev->name) + 1, path_len - 1)); // safe memcpy
            path[path_len - 1] = 0;                                            // null terminate
            return path;                                                       // return the path on success
        }
    }

    return NULL; // return NULL on failure
}

vfs_fs_node_t *devfs_open(struct vfs_fs_ops *fs, const char *path)
{
    // todo: parse the hierarchy
    device_t *dev;
    for (dev = device_list; dev; dev = dev->next)
    {
        if (strncmp(path, dev->name, strlen((char *)path)) == 0)
            break;
    }

    if (!dev)
    {
        log_error("failed to open device at %s", path);
        return NULL;
    }

    device_node_t *node = block_allocate(sizeof(device_node_t));

    // fill in the standard vfs header
    size_t path_len = strlen((char *)path);
    node->vfs_header.path_length = path_len;
    node->vfs_header.path = block_allocate(path_len);
    node->vfs_header.fs = fs;
    node->vfs_header.is_device = true;
    memcpy(node->vfs_header.path, path, path_len);

    node->device = dev; // remember the device

    return (vfs_fs_node_t *)node;
}

void devfs_close(vfs_fs_node_t *node)
{
    device_node_t *dev_node = (device_node_t *)node;

    if (dev_node->vfs_header.path)
        block_deallocate(dev_node->vfs_header.path);

    block_deallocate(dev_node);
}

void *devfs_read(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset)
{
    device_node_t *dev_node = (device_node_t *)node;
    if (!dev_node->device->read)
    {
        log_error("%s doesn't support read", dev_node->device->name);
        return buffer;
    }

    return dev_node->device->read(node, buffer, size, offset);
}

void *devfs_write(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset)
{
    device_node_t *dev_node = (device_node_t *)node;
    if (!dev_node->device->write)
    {
        log_error("%s doesn't support write", dev_node->device->name);
        return buffer;
    }

    return dev_node->device->write(node, buffer, size, offset);
}

void device_manager_init(void)
{
    // mount filesystem
    devfs.name = "devfs";
    devfs.name_length = 5;
    devfs.open = devfs_open;
    devfs.close = devfs_close;
    devfs.write = devfs_write;
    devfs.read = devfs_read;
    vfs_mount_fs("dev", &devfs);

    // populate using detected devices
    framebuffer_create_device();
    serial_create_device();
}