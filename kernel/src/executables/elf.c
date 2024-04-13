#define MODULE "elf"
#include <misc/logger.h>

#include <executables/elf.h>
#include <executables/elfabi.h>
#include <memory/physical/block_allocator.h>

bool elf_load_from(vfs_fs_node_t *node)
{
    // read the elf header
    Elf64_Ehdr *elf_header = block_allocate(sizeof(Elf64_Ehdr));
    node->fs->read(node, elf_header, sizeof(Elf64_Ehdr), 0);

    // validate the header
    if (memcmp(elf_header->e_ident, ELFMAG, 4) != 0)
    {
        log_error("failed to load executable with unknown signature");
        goto fail;
    }

    if (elf_header->e_ident[4] != 2)
    {
        log_error("failed to load executable with unsupported object size");
        goto fail;
    }

    if (elf_header->e_type != ET_EXEC)
    {
        log_error("failed to load non-executable object");
        goto fail;
    }

    if (elf_header->e_machine != EM_X86_64)
    {
        log_error("failed to load executable with unsupported target architecture");
        goto fail;
    }

    node->fs->close(node); // close the node
    return true;

fail:
    node->fs->close(node); // close the node
    return false;
}