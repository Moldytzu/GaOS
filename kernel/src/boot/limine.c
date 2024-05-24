#define MODULE "limine"
#include <misc/logger.h>
#include <arch/arch.h>

#include <devices/manager.h>
#include <boot/limine.h>
#include <memory/physical/page_allocator.h>

LIMINE_BASE_REVISION(1) // set latest revision

volatile struct limine_framebuffer_request kernel_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

volatile struct limine_hhdm_request kernel_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

volatile struct limine_memmap_request kernel_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

volatile struct limine_kernel_address_request kernel_kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

volatile struct limine_stack_size_request kernel_stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = PAGE,
};

volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0,
};

uint64_t kernel_hhdm_offset;

struct limine_file *limine_get_module(const char *path)
{
    size_t count = module_request.response->module_count;
    struct limine_file **modules = module_request.response->modules;

    for (size_t i = 0; i < count; i++)
    {
        struct limine_file *file = modules[i];
        char *fpath = file->path;

        if (*path != '/') // if the given path doesn't start with a delimiter
            fpath++;      // do not compare with it

        if (strncmp(fpath, path, strlen((char *)path)) == 0)
            return file;
    }

    log_error("failed to get module %s", path);

    return nullptr;
}

void limine_init()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false) // check if the bootloader supports what we request
        halt();

    kernel_hhdm_offset = kernel_hhdm_request.response->offset;
}

void limine_create_device()
{
    // create the root
    char *path = page_allocate(1);
    strcpy(path, "/boot/modules/");
    device_create_at(path, reserved, nullptr, nullptr);

    // create a device for each module if there exist any
    if (!module_request.response)
        return;

    int count = module_request.response->module_count;
    struct limine_file **modules = module_request.response->modules;

    for (int i = 0; i < count; i++)
    {
        struct limine_file *file = modules[i];
        strcpy(path + 14, file->path);
        device_create_at(path, module, nullptr, nullptr);
    }
}