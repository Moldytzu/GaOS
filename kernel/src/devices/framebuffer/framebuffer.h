#pragma once
#include <misc/libc.h>

typedef struct
{
    void *base;
    size_t width;
    size_t height;
    size_t pitch;
    size_t bpp;
} framebuffer_t;

typedef struct
{
    size_t x, y;
    uint32_t colour;
} cursor_t;

extern bool framebuffer_available;
extern framebuffer_t main_framebuffer;
extern cursor_t main_cursor;

void framebuffer_init();
void framebuffer_write_string(char *string);
void framebuffer_write_character(char c);

ifunc void framebuffer_plot_pixel(size_t x, size_t y, uint32_t colour)
{
    uint32_t *line = (uint32_t *)((uint64_t)main_framebuffer.base + y * main_framebuffer.pitch); // determine the base of the line we want to plot the pixel
    line[x] = colour;                                                                            // do the actual plot
}