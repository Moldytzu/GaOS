#pragma once
#include <misc/libc.h>
#include <devices/framebuffer/framebuffer.h>
#include <clock/clock.h>

#ifndef MODULE
#error logger has to have a MODULE name set
#endif

#define PREFIX_COLOUR 0xD0D0D0
#define DEBUG_COLOUR 0x3BADFF
#define INFO_COLOUR 0x10B010
#define WARN_COLOUR 0xD07000
#define ERROR_COLOUR 0xFF2020

void _log(uint32_t colour, const char *fmt, ...);

#define log_info(fmt, ...) _log(INFO_COLOUR, (fmt), ##__VA_ARGS__)
#define log_warn(fmt, ...) _log(WARN_COLOUR, (fmt), ##__VA_ARGS__)
#define log_error(fmt, ...) _log(ERROR_COLOUR, (fmt), ##__VA_ARGS__)