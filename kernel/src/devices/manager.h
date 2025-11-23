#pragma once
#include <misc/libc.h>
#include <filesystem/vfs.h>

typedef enum
{
    other,
    reserved,
    serial,
    framebuffer,
    timer,
    hmi, // human machine interface
    module,
    acpi_table,
} device_type_t;

struct device
{
    char *name;
    size_t name_length;
    device_type_t type;

    ssize_t (*read)(struct vfs_fs_node *node, void *buffer, size_t size);
    ssize_t (*write)(struct vfs_fs_node *node, void *buffer, size_t size);
    int (*initialise_node)(struct vfs_fs_node *node);

    struct device *parent;
    struct device *child;

    struct device *next;
    struct device *prev;
};

typedef struct device device_t;

void device_manager_init();
device_t *device_create_at(const char *path, device_type_t type, void *initialise_node, void *read, void *write);
char *device_get_by_type(device_type_t type, char *path, size_t path_len, uint64_t index);