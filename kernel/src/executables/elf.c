#define MODULE "elf"
#include <misc/logger.h>

#include <executables/elf.h>
#include <executables/elfabi.h>
#include <memory/physical/block_allocator.h>
#include <memory/physical/page_allocator.h>
#include <arch/arch.h>
#include <schedulers/task/task.h>

bool elf_load_from(vfs_fs_node_t *node)
{
    // read the elf header
    Elf64_Ehdr elf_header;
    node->fs->read(node, &elf_header, sizeof(Elf64_Ehdr), 0);

    // validate the header
    if (memcmp(elf_header.e_ident, ELFMAG, 4) != 0)
    {
        log_error("failed to load executable with unknown signature");
        goto fail;
    }

    if (elf_header.e_ident[4] != ELFDATA2MSB)
    {
        log_error("failed to load executable with unsupported object size");
        goto fail;
    }

    if (elf_header.e_type != ET_EXEC)
    {
        log_error("failed to load non-executable object");
        goto fail;
    }

    if (elf_header.e_machine != EM_X86_64)
    {
        log_error("failed to load executable with unsupported target architecture");
        goto fail;
    }

    // read the program headers to determine the smallest base address (useful when dealing with the stack)
    uint64_t base_address = -1;
    Elf64_Phdr program_header;
    for (int i = 0; i < elf_header.e_phnum; i++)
    {
        node->fs->read(node, &program_header, sizeof(Elf64_Phdr), elf_header.e_phoff + sizeof(Elf64_Phdr) * i);

        if (program_header.p_type != PT_LOAD)
            continue;

        if (program_header.p_vaddr < base_address)
            base_address = program_header.p_vaddr;
    }

    log_info("base of %s is %p", node->path, base_address);

    // read each program header in memory while mapping in the addressing space
    arch_page_table_t *page_table = arch_table_manager_new();

    // map the higher half
    for (int i = 0x100; i <= 0x1FF; i++)
        page_table->entries[i] = arch_bootstrap_page_table->entries[i];

    // read the program headers
    for (int i = 0; i < elf_header.e_phnum; i++)
    {
        node->fs->read(node, &program_header, sizeof(Elf64_Phdr), elf_header.e_phoff + sizeof(Elf64_Phdr) * i);

        if (program_header.p_type != PT_LOAD)
            continue;

        size_t memsize = program_header.p_memsz;
        uint64_t base = program_header.p_vaddr;
        for (uint64_t i = 0; i <= memsize; i += PAGE)
        {
            void *page = page_allocate(1);
            arch_table_manager_map(page_table, base + i, (uint64_t)page - kernel_hhdm_offset, TABLE_ENTRY_USER | TABLE_ENTRY_READ_WRITE);

            // calculate the remaining size to load
            long long load_size = (long long)program_header.p_filesz - (long long)i;
            if (load_size < -PAGE)
                continue;

            load_size = abs(load_size);
            load_size = min(load_size, PAGE); // clamp the value to page size

            node->fs->read(node, page, load_size, program_header.p_offset + i);
        }
    }

    // create a small stack for it
    uint64_t stack_base = base_address - PAGE; // we want it to be just under the executable
    arch_table_manager_map(page_table, stack_base, (uint64_t)page_allocate(1) - kernel_hhdm_offset, TABLE_ENTRY_USER | TABLE_ENTRY_READ_WRITE | TABLE_ENTRY_NO_EXECUTE);

    // create a task for it
    scheduler_task_t *new_task = task_scheduler_create(node->path);

#ifdef ARCH_x86_64
    new_task->state.cr3 = (uint64_t)page_table - kernel_hhdm_offset; // cr3 has to be a physical address
    new_task->state.rip = (uint64_t)elf_header.e_entry;              // point to the designated entry point
    new_task->state.rflags = 0x202;                                  // enable interrupts
    new_task->state.rsp = stack_base + PAGE;                         // point to the stack (on x86_64 the stack grows down)

    // point to userspace segements
    new_task->state.cs = 8 * 4 | 3;
    new_task->state.ss = 8 * 3 | 3;
#endif

    new_task->empty = false;

    node->fs->close(node); // close the node
    return true;

fail:
    node->fs->close(node); // close the node
    return false;
}