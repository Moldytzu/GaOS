#include <devices/framebuffer/framebuffer.h>
#include <devices/serial/serial.h>
#include <boot/limine.h>

bool framebuffer_available = false;
framebuffer_t main_framebuffer;

bool framebuffer_generate_structure_from_limine(framebuffer_t *f)
{
    if (kernel_framebuffer_request.response && kernel_framebuffer_request.response->framebuffer_count == 0) // check if we have a framebuffer
        return false;

    // translate limine's framebuffer structure to our format
    struct limine_framebuffer *fb = kernel_framebuffer_request.response->framebuffers[0];
    f->base = fb->address;
    f->width = fb->width;
    f->height = fb->height;
    f->pitch = fb->pitch;
    f->bpp = fb->bpp;

    return true;
}

void framebuffer_init()
{
    // generate the framebuffer structure
    if (!framebuffer_generate_structure_from_limine(&main_framebuffer))
    {
        serial_send_string("framebuffer: failed to detect any framebuffer\n");
        return;
    }

    framebuffer_available = true;

    // plot a pink line
    for (int i = 0; i < 300; i++)
        framebuffer_plot_pixel(i, i, 0xFF00FF);
}