#define MODULE "vfs"
#include <misc/logger.h>

#include <filesystem/vfs.h>
#include <memory/physical/block_allocator.h>
#include <arch/arch.h>

vfs_mount_point_t *mount_points;
arch_spinlock_t mount_lock;

void vfs_print_debug();

void vfs_mount_fs(const char *name, vfs_fs_ops_t *fs)
{
    // fixme: this should check if the name is already taken

    arch_spinlock_acquire(&mount_lock);

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

    arch_spinlock_release(&mount_lock);

    log_info("mounting %s on /%s", fs->name, name);
}

vfs_fs_node_t *vfs_open(const char *path)
{
    arch_spinlock_acquire(&mount_lock);

    if (path == NULL)
    {
        log_error("attempt to open NULL"); // todo: print caller's address
        return NULL;
    }

    size_t path_len = strlen((char *)path);

    // structure of a path is
    // /<filesystem name>/path/to/files
    // anything that isn't like this is invalid and should return NULL

    if (*path != '/')
    {
        log_error("failed to open \"%s\" with unknown root", path);
        return NULL;
    }

    // get the name of the filesystem targeted in the path
    size_t fsname_len;
    for (fsname_len = 1; path[fsname_len] != '/' && fsname_len < path_len; fsname_len++)
        ;
    fsname_len--; // ignore last /

    vfs_mount_point_t *mount = mount_points->next; // get first mount point
    while (mount)
    {
        if (strncmp(path + 1, mount->name, fsname_len) == 0) // compare the mount names
            break;                                           // and if they're identical break

        mount = mount->next; // else, continue comparing
    }

    if (!mount) // invalid filesystem
    {
        log_error("failed to open \"%s\" with unknown filesystem", path);
        return NULL;
    }

    arch_spinlock_release(&mount_lock);

    return mount->fs->open(mount->fs, path + fsname_len + 1 /*skip /<filesystem name>*/);
}

void vfs_print_debug()
{
    arch_spinlock_acquire(&mount_lock);

    vfs_mount_point_t *point = mount_points->next;
    while (point)
    {
        printk_serial("vfs: mount point: /%s/ on %s\n", point->name, point->fs->name);
        point = point->next;
    }

    arch_spinlock_release(&mount_lock);
}

void vfs_init()
{
    mount_points = block_allocate(sizeof(vfs_mount_point_t));
}