#define MODULE "vfs"
#include <misc/logger.h>
#include <misc/panic.h>

#include <filesystem/vfs.h>
#include <memory/physical/block_allocator.h>
#include <arch/arch.h>

vfs_mount_point_t *mount_points;
spinlock_t mount_lock;

void vfs_print_debug();

void vfs_mount_fs(const char *name, vfs_fs_ops_t *fs)
{
    // fixme: this should check if the name is already taken

    spinlock_acquire(&mount_lock);

    size_t name_len = strlen((char *)name);

    vfs_mount_point_t *new_mount = block_allocate(sizeof(vfs_mount_point_t)); // allocate a new mount point

    // copy the name we've been given
    new_mount->name_length = name_len;
    new_mount->name = block_allocate(name_len);
    memcpy(new_mount->name, name, name_len);

    // fill in the filesystem metadata
    new_mount->fs = fs;

    // push on the mount_points list
    vfs_mount_point_t *point = mount_points;
    while (point->next) // navigate to the last point
        point = point->next;

    point->next = new_mount;

    spinlock_release(&mount_lock);

    log_info("mounting %s on /%s", fs->name, name);
}

vfs_fs_node_t *vfs_open(const char *path, uint64_t mode)
{
    if (path == nullptr)
    {
        log_error("attempt to open nullptr"); // todo: print caller's address
        return (void *)-ENOENT;
    }

    size_t path_len = strlen((char *)path);

    // structure of a path is
    // /<filesystem name>/path/to/files
    // anything that isn't like this is invalid and should return an error

    if (*path != '/')
    {
        log_error("failed to open \"%s\" with unknown root", path);
        return (void *)-ENOENT;
    }

    // get the name of the filesystem targeted in the path
    size_t fsname_len;
    for (fsname_len = 1; path[fsname_len] != '/' && fsname_len < path_len; fsname_len++)
        ;
    fsname_len--; // ignore last /

    spinlock_acquire(&mount_lock);
    vfs_mount_point_t *mount = mount_points->next; // get first mount point
    while (mount)
    {
        if (strncmp(path + 1, mount->name, fsname_len) == 0) // compare the mount names
            break;                                           // and if they're identical break

        mount = mount->next; // else, continue comparing
    }
    spinlock_release(&mount_lock);

    if (!mount) // invalid filesystem
    {
        log_error("failed to open \"%s\" with unknown filesystem", path);
        return (void *)-ENOENT;
    }

    if (!mount->fs->open)
        panic("%s has no open callback", mount->fs->name);

    return mount->fs->open(mount->fs, path + fsname_len + 1 /*skip /<filesystem name>*/, mode);
}

void *vfs_read(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset)
{
    if (!node)
        panic("kernel bug: null vfs node");

    if (!node->fs->read)
    {
        log_error("%s has no read callback", node->fs->name);
        return buffer;
    }

    return node->fs->read(node, buffer, size, offset);
}

void *vfs_write(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset)
{
    if (!node)
        panic("kernel bug: null vfs node");

    if (!node->fs->write)
    {
        log_error("%s has no write callback", node->fs->name);
        return buffer;
    }

    return node->fs->write(node, buffer, size, offset);
}

void vfs_close(vfs_fs_node_t *node)
{
    if (!node)
        panic("kernel bug: null vfs node");

    if (!node->fs->close)
    {
        panic("%s has no close callback", node->fs->name);
        return;
    }
}

bool vfs_async_task_register(struct vfs_fs_node *node, io_task_t *task)
{
    if (!node->fs->async_task_register || !node->fs->async_task_unregister || !node->fs->async_task_update)
    {
        log_error("%s does not support async i/o", node->fs->name);
        return false;
    }

    node->fs->async_task_register(node, task);

    return true;
}

void vfs_async_task_update(struct vfs_fs_node *node, io_task_t *task)
{
    if (!node->fs->async_task_register || !node->fs->async_task_unregister || !node->fs->async_task_update)
    {
        log_error("%s does not support async i/o", node->fs->name);
        return;
    }

    node->fs->async_task_update(node, task);
}

void vfs_async_task_unregister(struct vfs_fs_node *node, io_task_t *task)
{
    if (!node->fs->async_task_register || !node->fs->async_task_unregister || !node->fs->async_task_update)
    {
        log_error("%s does not support async i/o", node->fs->name);
        return;
    }

    node->fs->async_task_unregister(node, task);
}

void vfs_print_debug()
{
    spinlock_acquire(&mount_lock);

    vfs_mount_point_t *point = mount_points->next;
    while (point)
    {
        printk_serial("vfs: mount point: /%s/ on %s\n", point->name, point->fs->name);
        point = point->next;
    }

    spinlock_release(&mount_lock);
}

void vfs_init()
{
    mount_points = block_allocate(sizeof(vfs_mount_point_t));
}

void vfs_dirname(const char *path, char *dirname, size_t max_len)
{
    // get parent directory of path
    size_t max_offset = strlen((char *)path);

    if (path[max_offset] == '/') // skip if this is a folder
        max_offset--;

    while (path[max_offset] != '/' && max_offset != 0) // decrease max_offset until it points to a directory
        max_offset--;

    memcpy(dirname, path, min(max_len, max_offset));
}

void vfs_basename(const char *path, char *basename, size_t max_len)
{
    // get basename of path
    size_t max_offset = strlen((char *)path);
    size_t base_offset = max_offset;

    while (base_offset != 0 && path[base_offset] != '/') // count down until we encounter the first character or a delimiter
        base_offset--;

    if (path[base_offset] == '/')
        base_offset++;

    memcpy(basename, path + base_offset, min(max_len, max_offset - base_offset));
}

size_t vfs_sanatise_path(char *path)
{
    // clean up path from excesive delimiters, returns the new string length

    if (*path == 0) // don't bother with empty paths
        return 0;

    int index = 1; // comparisons are made -1 from index, thus we have to start from 1
    while (path[index] != 0)
    {
        if (path[index - 1] == '/' && path[index] == '/') // double delimiters
        {
            /// remove the character at [index] and shift left the whole string
            for (int i = index; path[i] != 0; i++)
                path[i] = path[i + 1];
        }
        else
            index++;
    }

    return index;
}