# This is the name that our final app executable will have.
APP :=  $(notdir $(shell pwd)).elf

# This is the prefix of our custom cross-compiler
# fixme: replace this with a userspace toolchain
PREFIX := $(TOOLCHAIN_BASE)/kernel/bin/$(OUTPUT_ARCH)-elf-

# Default compiler/linker
CC := $(PREFIX)gcc
LD := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy

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
    -fno-pie \
	-fno-pic \
    -march=nocona \
    -mno-red-zone \
    -Wno-unused-function

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
all: bin/$(APP)

# Link rules for the final app executable.
bin/$(APP): linker.ld $(OBJ)
	@ mkdir -p "$$(dirname $@)"
	@ $(LD) $(OBJ) $(LDFLAGS) -o $@
	@ echo -e '\tLD $@'
	@ cp $@ $(BASE)/root/initrd/

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
obj/%.c.o: src/%.c
	@ mkdir -p "$$(dirname $@)"
	@ echo $(CC)
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