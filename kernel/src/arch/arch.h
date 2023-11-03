#pragma once

#ifdef ARCH_x86_64
#include <arch/x86_64/arch.h>
#else
#error Unknown architecture selected
#endif