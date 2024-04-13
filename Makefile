override IMAGE_NAME := disk
OUTPUT_ARCH = x86_64
CORES = $(shell nproc)
BASE = $(shell pwd)
TOOLCHAIN_BASE = $(shell pwd)/toolchain/$(OUTPUT_ARCH)/

# make the architecture available in all makefiles
export OUTPUT_ARCH
export TOOLCHAIN_BASE
export BASE

QEMU_FLAGS = -m 2G -serial stdio -smp 2
QEMU_LOG = -D qemu.out -d int -no-reboot -no-shutdown
QEMU_ACCELERATED = 
QEMU_DEBUG = $(QEMU_LOG) -smp 1 -s -S
GDB_FLAGS ?= -tui -q -x gdb.script

# architecture-specific flags
ifeq ($(OUTPUT_ARCH),x86_64)
	QEMU_FLAGS += -M q35,smm=off
	QEMU_ACCELERATED += --enable-kvm -cpu host
endif

# apps
APPS = $(wildcard ./apps/*/.)

.PHONY: all
all: install_hdd

.PHONY: run
run: all
	qemu-system-$(OUTPUT_ARCH) $(QEMU_FLAGS) -hda $(IMAGE_NAME).hdd -boot c

run-accel: all
	qemu-system-$(OUTPUT_ARCH) $(QEMU_FLAGS) -hda $(IMAGE_NAME).hdd -boot c $(QEMU_ACCELERATED)

run-gdb: all
	qemu-system-$(OUTPUT_ARCH) $(QEMU_FLAGS) $(QEMU_DEBUG) -hda $(IMAGE_NAME).hdd -boot c &
	gdb-multiarch $(GDB_FLAGS) kernel/bin/kernel.elf
	pkill -f qemu-system-$(OUTPUT_ARCH)
	reset

run-uefi: all ovmf
	qemu-system-$(OUTPUT_ARCH) $(QEMU_FLAGS) -bios ovmf/OVMF.fd -hda $(IMAGE_NAME).hdd

run-log: all
	qemu-system-$(OUTPUT_ARCH) $(QEMU_FLAGS) $(QEMU_LOG) -hda $(IMAGE_NAME).hdd -boot c

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1
	$(MAKE) -C limine -j$(CORES)

.PHONY: kernel
kernel:
	$(MAKE) -C kernel -j$(CORES)

FORCE:

$(APPS): FORCE
	$(MAKE) -C $@

$(IMAGE_NAME).hdd: limine
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	sgdisk $(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
	./limine/limine bios-install $(IMAGE_NAME).hdd

install_hdd: $(IMAGE_NAME).hdd kernel $(APPS)
	cd $(BASE)/root/initrd/ && tar -cf ../initrd.tar .
	mformat -i $(IMAGE_NAME).hdd@@1M
	mmd -i $(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M root/initrd.tar kernel/bin/kernel.elf limine.cfg limine/limine-bios.sys ::/
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT

.PHONY: clean
clean:
	$(MAKE) -C kernel clean
	for dir in $(APPS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf $(IMAGE_NAME).hdd
	$(MAKE) -C kernel distclean

deps-debian:
	@echo Execution as root might be required for this target
	@echo Use sudo if errors occur
	@echo 
	apt install build-essential fdisk nasm mtools gdisk gdb-multiarch # kernel
	apt install libbison-dev flex libgmp-dev libmpfr-dev libmpc-dev texinfo gcc g++ make # gcc + binutils

.PHONY: toolchain
toolchain:
	bash ./toolchain/build_kernel_toolchain.sh $(OUTPUT_ARCH)