#pragma once
#include <misc/libc.h>
#include <io/queue.h>

// POSIX oflags
#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_APPEND (1 << 2)
#define O_CREAT (1 << 3)
#define O_DSYNC (1 << 4)
#define O_EXCL (1 << 5)
#define O_NOCTTY (1 << 6)
#define O_NONBLOCK (1 << 7)
#define O_RSYNC (1 << 8)
#define O_SYNC (1 << 9)
#define O_TRUNC (1 << 10)

struct vfs_fs_node
{
    char *path;
    size_t path_length;

    uint64_t mode;
    size_t seek_position;
    size_t max_seek_position;

    bool is_device;

    struct vfs_fs_ops *fs;
};

typedef struct vfs_fs_node vfs_fs_node_t;

struct vfs_fs_ops
{
    char *name;
    size_t name_length;

    struct vfs_fs_node *(*open)(struct vfs_fs_ops *fs, const char *path, uint64_t mode);
    ssize_t (*read)(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset);
    ssize_t (*write)(struct vfs_fs_node *node, void *buffer, size_t size, size_t offset);
    void (*close)(struct vfs_fs_node *node);
    struct vfs_fs_node *(*dup)(struct vfs_fs_node *node);
    void (*async_task_register)(struct vfs_fs_node *node, io_task_t *task);
    void (*async_task_update)(struct vfs_fs_node *node, io_task_t *task);
    void (*async_task_unregister)(struct vfs_fs_node *node, io_task_t *task);
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

void vfs_init();
void vfs_mount_fs(const char *name, vfs_fs_ops_t *fs);
vfs_fs_node_t *vfs_open(const char *path, uint64_t mode);
ssize_t vfs_read(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset);
ssize_t vfs_write(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset);
bool vfs_async_task_register(struct vfs_fs_node *node, io_task_t *task);
void vfs_async_task_update(struct vfs_fs_node *node, io_task_t *task);
void vfs_async_task_unregister(struct vfs_fs_node *node, io_task_t *task);
void vfs_close(vfs_fs_node_t *node);
vfs_fs_node_t *vfs_dup(vfs_fs_node_t *node);
void vfs_dirname(const char *path, char *dirname, size_t max_len);
void vfs_basename(const char *path, char *basename, size_t max_len);
size_t vfs_sanatise_path(char *path);
void vfs_print_debug();