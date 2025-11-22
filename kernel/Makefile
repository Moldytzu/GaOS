# This is the name that our final kernel executable will have.
KERNEL := kernel.elf

# This is the prefix of our custom cross-compiler
PREFIX := $(TOOLCHAIN_BASE)/kernel/bin/$(OUTPUT_ARCH)-elf-

# Default compiler/linker
CC := $(PREFIX)gcc
LD := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy

# User controllable flags.
CFLAGS := -g -Og -pipe
CPPFLAGS :=
NASMFLAGS := -F dwarf -g
LDFLAGS :=

# Internal C flags that should not be changed by the user.
CFLAGS += \
    -Wall \
    -Wextra \
    -Werror \
    -Wlogical-op \
    -Waggregate-return \
    -Wstrict-prototypes \
    -fno-omit-frame-pointer \
    -std=gnu2x \
    -ffreestanding \
    -fno-stack-protector \
    -fstack-protector-all \
    -fsanitize=undefined \
    -fno-lto \
    -fno-pic \
    -fpie \
    -march=x86-64 \
    -mgeneral-regs-only \
    -mno-red-zone \
    -Wno-unused-function \
    -Wno-frame-address

# Internal C preprocessor flags that should not be changed by the user.
CPPFLAGS := \
    -Isrc/ \
    $(CPPFLAGS) \
    -MMD \
    -MP \
    -D ARCH_$(OUTPUT_ARCH)

# Internal linker flags that should not be changed by the user.
LDFLAGS += \
    -m elf_$(OUTPUT_ARCH) \
    -nostdlib \
    -static \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T linker.ld
    # -pie \ # disabled for debugging 

# Internal nasm flags that should not be changed by the user.
NASMFLAGS += \
    -Wall \
    -f elf64 \
    -I ./src/ \
    -I ./src/arch/x86_64/

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell cd src && find -L * -type f -name '*.c')
override ASFILES := $(shell cd src && find -L * -type f -name '*.S')
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm')
override OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target.
.PHONY: all
all: bin/$(KERNEL)

# Link rules for the final kernel executable.
bin/$(KERNEL): linker.ld $(OBJ)
	@ mkdir -p "$$(dirname $@)"
	@ $(OBJCOPY) -I binary -O elf64-x86-64 -B i386 kfont.psf obj/kfont.o
	@ $(LD) $(OBJ) obj/kfont.o $(LDFLAGS) -o $@
	@ echo -e '\tLD $@'

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
obj/%.c.o: src/%.c src/limine.h
	@ mkdir -p "$$(dirname $@)"
	@ $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	@ echo -e '\tCC $<'

# Compilation rules for *.S files.
obj/%.S.o: src/%.S
	@ mkdir -p "$$(dirname $@)"
	@ $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	@ echo -e '\tCC $<'

# Compilation rules for *.asm (nasm) files.
obj/%.asm.o: src/%.asm
	@ mkdir -p "$$(dirname $@)"
	@ nasm $(NASMFLAGS) $< -o $@
	@ echo -e '\tNASM $<'

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin obj

.PHONY: distclean
distclean: clean
	rm -f src/limine.h