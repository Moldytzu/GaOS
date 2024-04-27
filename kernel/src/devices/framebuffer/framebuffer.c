#define MODULE "framebuffer"
#include <misc/logger.h>

#include <devices/framebuffer/framebuffer.h>
#include <devices/serial/serial.h>
#include <boot/limine.h>
#include <arch/arch.h>

#define PSF2_MAGIC "\x72\xB5\x4A\x86"

typedef struct __attribute__((packed))
{
    uint8_t magic[4];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t length;
    uint32_t glyph_size;
    uint32_t height;
    uint32_t width;
} psf2_header_t;

// globals
bool framebuffer_available = false;
framebuffer_t main_framebuffer;
cursor_t main_cursor;

// font
extern uint8_t _binary_kfont_psf_start[];
static const uint8_t *kfont_ptr = (uint8_t *)_binary_kfont_psf_start;
static psf2_header_t *font;
static uint8_t *font_glyphs;

arch_spinlock_t framebuffer_spinlock;

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

void framebuffer_plot_character(char c, size_t x, size_t y, uint32_t colour)
{
    uint8_t *character = font_glyphs + c * font->glyph_size; // first byte of the glyph
    uint8_t line_pitch = font->glyph_size / font->height;    // bytes per line

    for (uint32_t yy = 0; yy < font->height; yy++)
    {
        for (uint32_t xx = 0; xx < font->width; xx++)
        {
            // here we index the character base with the pitch and the offset
            // since xx can span on multiple bytes, we have to divide by 8
            // after that we create a bitmask from the same xx but modulo 8 (same reason as on top but we need the bit index)
            // we and them and if we have a positive result, we can plot the pixel
            if ((character[yy * line_pitch + xx / bitsof(uint8_t)] & (0b10000000 >> xx % bitsof(uint8_t))))
                framebuffer_plot_pixel(x + xx, y + yy, colour);
        }
    }
}

void framebuffer_write_character(char c)
{
    arch_spinlock_acquire(&framebuffer_spinlock);

    // handle end of the line
    if (main_cursor.x + font->width >= main_framebuffer.width || c == '\n')
    {
        main_cursor.y += font->height;
        main_cursor.x = 0;
    }

    if (c == '\n') // don't plot \n
    {
        arch_spinlock_release(&framebuffer_spinlock);
        return;
    }

    // we're too low in the buffer
    if (main_cursor.y + font->height >= main_framebuffer.height)
    {
        // reset the cursor
        main_cursor.y = 0;
        main_cursor.x = 0;

        memset(main_framebuffer.base, 0, main_framebuffer.height * main_framebuffer.pitch); // clear the framebuffer
    }

    framebuffer_plot_character(c, main_cursor.x, main_cursor.y, main_cursor.colour); // draw the character
    main_cursor.x += font->width;                                                    // increase the coordonate

    arch_spinlock_release(&framebuffer_spinlock);
}

void framebuffer_write_string(char *string)
{
    while (*string)
        framebuffer_write_character(*string++);
}

void framebuffer_init(void)
{
    // generate the framebuffer structure
    if (!framebuffer_generate_structure_from_limine(&main_framebuffer))
    {
        serial_send_string("framebuffer: failed to detect any framebuffer\n");
        return;
    }

    if (main_framebuffer.bpp != 32)
    {
        serial_send_string("framebuffer: unsupported bit-depth\n");
        return;
    }

    // validate font
    font = (psf2_header_t *)kfont_ptr;
    font_glyphs = (uint8_t *)((uint64_t)font + font->header_size);
    if (memcmp(font->magic, PSF2_MAGIC, 4) != 0)
    {
        serial_send_string("framebuffer: failed to validate the embeded font\n");
        return;
    }

    // set up initial cursor metadata
    main_cursor.x = main_cursor.y = 0; // left-top
    main_cursor.colour = 0xFFFFFF;     // white

    framebuffer_available = true;

    log_info("initialised");
}