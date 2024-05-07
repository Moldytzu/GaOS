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

void list_devices_layer(device_t *list, int depth)
{
    char *type_string[] = {"other", "serial", "framebuffer", "timer", "hmi"};
    for (device_t *dev = list; dev; dev = dev->next)
    {
        for (int i = 0; i < depth; i++)
            printk_serial("-- ");

        printk_serial("%s\n", dev->name, type_string[dev->type]);

        if (dev->child)
            list_devices_layer(dev->child, depth + 1);
    }
}

void list_devices(void)
{
    printk_serial("listing devices:\n");
    list_devices_layer(device_list, 0);
}

device_t *allocate_device(char *path, device_type_t type, void *read, void *write)
{
    device_t *dev = block_allocate(sizeof(device_t));
    size_t path_len = strlen(path);
    dev->name = block_allocate(path_len);
    dev->name_length = path_len;
    dev->read = read;
    dev->write = write;
    dev->type = type;
    memcpy(dev->name, path, path_len);

    return dev;
}

device_t *allocate_dummy(void)
{
    return allocate_device(".", reserved, NULL, NULL);
}

char *device_generate_path_of(device_t *device, char *path, size_t path_len)
{
    // determine the depth by parsing the list parents
    size_t depth = 0;
    device_t *current_list = device;
    for (depth = 0; current_list; current_list = current_list->parent, depth++)
        ;

    device_t **hierarchy = block_allocate(depth * sizeof(device_t *));
    current_list = device;
    // create the hierarchy by parsing the list parents
    for (size_t i = 0; current_list; current_list = current_list->parent, i++)
        hierarchy[i] = current_list;

    path_len--; // reserve one byte for null-termination

    // create the path from the parsed hierarchy
    size_t offset = 0;
    for (int64_t i = depth - 1; i >= 0; i--)
    {
        char *name = hierarchy[i]->name;

        if (path_len <= offset) // check for overflow
            break;

        // write the delimiter
        path[offset] = '/';
        offset++;

        if (path_len <= offset) // check for overflow again
            break;

        memcpy(path + offset, name, min(strlen(name), path_len - offset)); // copy the name

        offset += strlen(name);
    }

    // null-terminate the string
    path[offset] = 0;

    block_deallocate(hierarchy);
    return path;
}

device_t *device_create_at(char *path, device_type_t type, void *read, void *write)
{
    // the basic idea of this algorithm is to split the given path in a hierachy of directories starting from the root (the top layer)
    // then, for each directory name, search in the list of the current level
    // if it exists, set the list to its child
    // else, try again, but with the path set to the wanted directory
    // when we arrive to the deepest directory or file, create a device in the list we found

    log_info("adding device %s", path);

    char *path_orig = path;       // original path pointer
    size_t total_path_offset = 0; // counter to all the offsets (useful when determining the parent directory)

    // skip '/' if needed
    if (*path == '/')
    {
        path++;
        total_path_offset++;
    }

    device_t *list = device_list;

    while (*path) // iterate over all of the directory fragments in the path
    {
        // determine the length of the next directory in the hierarchy
        size_t dir_len;
        for (dir_len = 0; path[dir_len] != '/' && path[dir_len] != 0; dir_len++)
            ;

        if (path[dir_len] == '/' && path[dir_len + 1] != 0) // there is another layer deep
        {
            // go deeper
            char *name = block_allocate(dir_len); // allocate a copy of the directory name
            memcpy(name, path, dir_len);

            // find the directory in the list
            device_t *dev = list;
            while (dev)
            {
                if (dir_len == dev->name_length && strncmp(name, dev->name, dir_len) == 0) // found the directory
                {
                    if (!dev->child) // initialise the list if needed
                        dev->child = allocate_dummy();
                    dev->child->parent = dev;

                    list = dev->child;
                    break;
                }
                dev = dev->next;
            }

            block_deallocate(name); // deallocate the copy

            path += dir_len; // skip the directory name
            total_path_offset += dir_len;

            if (*path == '/') // skip the delimiter if needed
            {
                path++;
                total_path_offset++;
            }

            if (!dev) // if the search failed
            {
                // create the parent directory
                char *parent = block_allocate(total_path_offset);
                memcpy(parent, path_orig, total_path_offset - 1 /*skips '/'*/);

                device_t *new = device_create_at(parent, reserved, NULL, NULL); // create the parent directory
                list = new->child = allocate_dummy();                           // create the child list
                new->child->parent = new;                                       // set the child accordingly
                block_deallocate(parent);
            }
        }
        else
        {
            // stop here
            bool create_as_dir = false;
            if (path[strlen(path) - 1] == '/')
                create_as_dir = true;

            if (create_as_dir)
                path[strlen(path) - 1] = 0; // remove the delimiter if the path is a directory

            // get last device in this list while checking for duplicates
            device_t *last = NULL, *current = list;
            while (current)
            {
                if (strncmp(current->name, path, strlen(path)) == 0)
                {
                    // oops.., already exists
                    log_error("failed to add %s because it already exists", path_orig);
                    return NULL;
                }

                if (current)
                    last = current;
                current = current->next;
            }

            // create the device
            last->next = allocate_device(path, type, read, write);
            last->next->parent = last->parent;
            last = last->next;

            if (create_as_dir) // create the child list if this is a directory
                last->child = allocate_dummy();

            return last;
        }
    }

    return NULL;
}

char *device_get_by_type_recursive(device_t *list, device_type_t type, char *path, size_t path_len, uint64_t *index, size_t depth)
{
    for (device_t *dev = list; dev; dev = dev->next)
    {
        if (dev->type == type && (*index)-- == 0) // check for the type and index
            return device_generate_path_of(dev, path, path_len);

        if (dev->child) // if the device has children
        {
            char *ret = device_get_by_type_recursive(dev->child, type, path, path_len, index, depth + 1); // try and search there
            if (ret)                                                                                      // if the search succeded
                return ret;                                                                               // return the path
        }
    }
    return NULL;
}

char *device_get_by_type(device_type_t type, char *path, size_t path_len, uint64_t index)
{
    // recursively parse the tree to find the device
    return device_get_by_type_recursive(device_list, type, path, path_len, &index, 0);
}

vfs_fs_node_t *devfs_open(struct vfs_fs_ops *fs, const char *path)
{
    char *path_orig = (char *)path; // original path pointer

    // skip '/' if needed
    if (*path == '/')
        path++;

    device_t *list = device_list;

    while (*path) // iterate over all of the directory fragments in the path
    {
        // determine the length of the next directory in the hierarchy
        size_t dir_len;
        for (dir_len = 0; path[dir_len] != '/' && path[dir_len] != 0; dir_len++)
            ;

        if (path[dir_len] == '/' && path[dir_len + 1] != 0) // there is another layer deep
        {
            // go deeper
            char *name = block_allocate(dir_len); // allocate a copy of the directory name
            memcpy(name, path, dir_len);

            // find the directory in the list
            device_t *dev = list;
            while (dev)
            {
                if (dir_len == dev->name_length && strncmp(name, dev->name, dir_len) == 0) // found the directory
                {
                    if (!dev->child) // initialise the list if needed
                        dev->child = allocate_dummy();
                    dev->child->parent = dev;

                    list = dev->child;
                    break;
                }
                dev = dev->next;
            }

            block_deallocate(name); // deallocate the copy

            path += dir_len; // skip the directory name

            if (*path == '/') // skip the delimiter if needed
                path++;

            if (!dev)  // if the search failed
                break; // handle the error
        }
        else
        {
            // we're at the lowest point, now find the device
            device_t *dev = list;
            while (dev)
            {
                if (dir_len == dev->name_length && strncmp(path, dev->name, dir_len) == 0) // found the device
                    break;
                dev = dev->next;
            }

            if (!dev)  // if not found,
                break; // handle the error

            device_node_t *node = block_allocate(sizeof(device_node_t));

            // fill in the standard vfs header
            size_t path_len = strlen((char *)path_orig);
            node->vfs_header.path_length = path_len;
            node->vfs_header.path = block_allocate(path_len);
            node->vfs_header.fs = fs;
            node->vfs_header.is_device = true;
            memcpy(node->vfs_header.path, path_orig, path_len);

            node->device = dev;

            return (vfs_fs_node_t *)node;
        }
    }

    log_error("failed to open device %s", path_orig);
    return NULL;
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

    // create a dummy device to be the head
    device_list = allocate_dummy();

    // populate using detected devices
    framebuffer_create_device();
    serial_create_device();

    list_devices();
}