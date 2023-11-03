# This is the name that our final kernel executable will have.
KERNEL := kernel.elf

# This is the prefix of our custom cross-compiler
PREFIX := ~/cross_compiler/bin/$(OUTPUT_ARCH)-elf-
# fixme: this could be moved here, in a toolchain folder

# Default compiler/linker
CC := $(PREFIX)gcc
LD := $(PREFIX)ld

# User controllable flags.
CFLAGS := -g -O2 -pipe
CPPFLAGS :=
NASMFLAGS := -F dwarf -g
LDFLAGS :=

# Internal C flags that should not be changed by the user.
CFLAGS += \
    -Wall \
    -Wextra \
    -Werror \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fPIE \
    -march=nocona \
    -mno-red-zone

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
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T linker.ld

# Internal nasm flags that should not be changed by the user.
NASMFLAGS += \
    -Wall \
    -f elf64

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

src/limine.h:
	curl -Lo $@ https://github.com/limine-bootloader/limine/raw/trunk/limine.h

# Link rules for the final kernel executable.
bin/$(KERNEL): linker.ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(LD) $(OBJ) $(LDFLAGS) -o $@

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
obj/%.c.o: src/%.c src/limine.h
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj/%.S.o: src/%.S
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
obj/%.asm.o: src/%.asm
	mkdir -p "$$(dirname $@)"
	nasm $(NASMFLAGS) $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin obj

.PHONY: distclean
distclean: clean
	rm -f src/limine.h