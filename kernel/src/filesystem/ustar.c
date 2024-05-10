#define MODULE "ustar"
#include <misc/logger.h>

#include <filesystem/ustar.h>
#include <filesystem/vfs.h>
#include <memory/physical/page_allocator.h>
#include <memory/physical/block_allocator.h>
#include <arch/arch.h>

#include <boot/limine.h>

#define TAR_BLOCK_SIZE 512
#define TAR_HEADER_SIZE TAR_BLOCK_SIZE

pstruct
{
    char name[100];
    uint64_t mode;
    uint64_t uid;
    uint64_t gid;
    char size[12];
    char mktime[12];
    uint64_t chksum;
    uint8_t typeflag;
    char linkname[100];
    char magic[6];
    uint16_t version;
    char uname[32];
    char gname[32];
    uint64_t devmajor;
    uint64_t devminor;
    char prefix[155];
}
ustar_header_t;

typedef struct
{
    vfs_fs_node_t vfs_header;
    ustar_header_t *ustar_header;
} ustar_node_t;

ustar_header_t *initrd;

size_t parse_size_of(ustar_header_t *header)
{
    size_t size = 0;

    for (int i = 0; i < 12; i++)
    {
        if (!header->size[i] || header->size[i] == ' ') // handle improper characters
            continue;
        size *= 8;
        size += header->size[i] - '0';
    }

    return size;
}

ustar_header_t *ustar_open_header(const char *path)
{
    ustar_header_t *header = (ustar_header_t *)((uint64_t)initrd + TAR_BLOCK_SIZE); // point to the first header after the root

    while (true)
    {
        if (header->name[0] == '\0')
            break;

        size_t size = parse_size_of(header); // get the size

        // real_size = align_to_next_block_size(<tar reported size> + <tar block size>)
        size_t real_size = size + TAR_HEADER_SIZE;

        if (real_size % TAR_BLOCK_SIZE)
            real_size += 512 - (real_size % TAR_BLOCK_SIZE);

        if (strncmp(path, header->name + 1 /*skip .*/, strlen((char *)path)) == 0) // find exact match of path
            return header;

        header = (ustar_header_t *)((uint64_t)header + real_size); // get next header
    }

    log_error("failed to open header with path %s", path);
    return nullptr;
}

vfs_fs_node_t *ustar_open(struct vfs_fs_ops *fs, const char *path, uint64_t mode)
{
    used(mode);
    ustar_node_t *node = block_allocate(sizeof(ustar_node_t));
    ustar_header_t *header = ustar_open_header(path);

    if (header == nullptr)
        return nullptr;

    // fill in vfs header with path
    size_t path_len = strlen((char *)path);
    node->vfs_header.path_length = path_len;
    node->vfs_header.path = block_allocate(path_len);
    node->vfs_header.fs = fs;
    memcpy(node->vfs_header.path, path, path_len);

    node->ustar_header = header; // save ustar header for later

    return (vfs_fs_node_t *)node;
}

void *ustar_read(vfs_fs_node_t *node, void *buffer, size_t size, size_t offset)
{
    ustar_node_t *ustar_node = (ustar_node_t *)node;
    size_t node_size = parse_size_of(ustar_node->ustar_header);
    size = min(node_size, size);

    if (offset >= node_size) // invalid offset
    {
        log_error("invalid offset %d when reading %s", offset, node->path);
        return nullptr;
    }

    if (offset + size >= node_size) // properly map size to fit the file
        size = node_size - offset;

    log_info("reading %d bytes from %s + %d offset", size, node->path, offset);

    void *contents = (void *)((uint64_t)ustar_node->ustar_header + TAR_HEADER_SIZE);

    return memcpy(buffer, (void *)((uint64_t)contents + offset), size);
}

void ustar_close(vfs_fs_node_t *node)
{
    ustar_node_t *ustar_node = (ustar_node_t *)node;

    if (ustar_node->vfs_header.path)
        block_deallocate(ustar_node->vfs_header.path);

    block_deallocate(ustar_node);
}

void ustar_debug_print()
{
    ustar_header_t *header = (ustar_header_t *)((uint64_t)initrd + TAR_HEADER_SIZE);

    // display all headers in the file
    while (true)
    {
        if (header->name[0] == '\0')
            break;

        size_t size = parse_size_of(header); // get the size

        // real_size = align_to_next_block_size(<tar reported size> + <tar block size>)
        // the header will always occupy a block!
        size_t real_size = size + TAR_BLOCK_SIZE;

        if (real_size % TAR_BLOCK_SIZE)
            real_size += 512 - (real_size % TAR_BLOCK_SIZE);

        log_info("file %s with %d bytes (%d bytes on disk)", header->name, size, real_size);

        header = (ustar_header_t *)((uint64_t)header + real_size); // get next header
    }
}

vfs_fs_ops_t ustar;

void ustar_init()
{
    // todo: we should create a device namespace
    // that can be called to get the required module
    // to use as initrd

    // open initrd file
    struct limine_file *initrd_file = limine_get_module("/initrd.tar");
    if (initrd_file == nullptr)
        return;
    initrd = initrd_file->address;

    // map in page table
    for (size_t i = 0; i <= initrd_file->size; i += 4096)
        arch_table_manager_map(arch_bootstrap_page_table, (uint64_t)initrd + i, (uint64_t)initrd + i - kernel_hhdm_offset, TABLE_ENTRY_READ_WRITE);

    // instantiate the filesystem
    ustar.close = ustar_close;
    ustar.open = ustar_open;
    ustar.read = ustar_read;
    ustar.name = "ustar";
    ustar.name_length = 5;

    vfs_mount_fs("initrd", &ustar); // mount it

    ustar_debug_print();
}