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

extern bool framebuffer_available;
extern framebuffer_t main_framebuffer;

void framebuffer_init();

ifunc void framebuffer_plot_pixel(size_t x, size_t y, uint32_t colour)
{
    uint32_t *line = (uint32_t *)((uint64_t)main_framebuffer.base + y * main_framebuffer.pitch); // determine the base of the line we want to plot the pixel
    line[x] = colour;                                                                            // do the actual plot
}