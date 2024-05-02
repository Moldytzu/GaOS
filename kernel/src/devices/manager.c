#define MODULE "device_manager"
#include <misc/logger.h>

#include <devices/manager.h>
#include <filesystem/vfs.h>
#include <memory/physical/block_allocator.h>

struct device
{
    char *name;
    size_t name_length;

    void *(*read)(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset);
    void *(*write)(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset);

    struct device *parent;
    struct device *child;

    struct device *next;
    struct device *prev;
};

typedef struct device device_t;

typedef struct
{
    vfs_fs_node_t vfs_header;
    device_t *device;
} device_node_t;

vfs_fs_ops_t devfs;
device_t *device_list = NULL;

void list_devices(void)
{
    for (device_t *dev = device_list; dev; dev = dev->next)
    {
        printk_serial("device: %s\n", dev->name);
    }
}

device_t *device_create_at(char *path, void *read, void *write)
{
    log_info("adding device %s", path);

    // todo: parse the hierarchy in path
    device_t *dev = block_allocate(sizeof(device_t));
    size_t path_len = strlen(path);
    dev->name = block_allocate(path_len);
    dev->name_length = path_len;
    dev->read = read;
    dev->write = write;
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

    // todo: do device detection
}