#define MODULE "limine"
#include <misc/logger.h>
#include <arch/arch.h>

#include <devices/manager.h>
#include <boot/limine.h>
#include <memory/physical/page_allocator.h>

static const uint64_t ____base_revision[] = LIMINE_BASE_REVISION(4);

volatile struct limine_framebuffer_request kernel_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_hhdm_request kernel_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_memmap_request kernel_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_executable_address_request kernel_executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_stack_size_request kernel_stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .stack_size = PAGE,
};

volatile struct limine_mp_request kernel_mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0,
};

volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
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
        {
            while (*fpath && *fpath == '/') // ignore it
                fpath++;
        }

        if (strncmp(fpath, path, strlen((char *)path)) == 0)
            return file;
    }

    log_error("failed to get module %s", path);

    return nullptr;
}

void limine_init()
{
    used(____base_revision);

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