#pragma once
#include <misc/libc.h>

struct vfs_fs_node
{
    char *path;
    size_t path_length;

    struct vfs_fs_ops *fs;
};

typedef struct vfs_fs_node vfs_fs_node_t;

struct vfs_fs_ops
{
    char *name;
    size_t name_length;

    struct vfs_fs_node *(*open)(struct vfs_fs_ops *fs, const char *path);
    void *(*read)(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset);
    void (*close)(struct vfs_fs_node *node);
};

typedef struct vfs_fs_ops vfs_fs_ops_t;

struct vfs_mount_point
{
    char *name;
    size_t name_length;

    vfs_fs_ops_t *fs;

    struct vfs_mount_point *next;
    struct vfs_mount_point *prev;
};

typedef struct vfs_mount_point vfs_mount_point_t;

void vfs_init(void);
void vfs_mount_fs(const char *name, vfs_fs_ops_t *fs);
vfs_fs_node_t *vfs_open(const char *path);