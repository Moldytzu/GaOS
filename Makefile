override IMAGE_NAME := disk

# This is the prefix of our custom cross-compiler
PREFIX := ~/cross_compiler/bin/x86_64-elf- # fixme: this could be moved here, in a toolchain folder

# Default compiler/linker
CC := $(PREFIX)gcc
LD := $(PREFIX)ld

.PHONY: all
all: install_hdd

.PHONY: run
run: all
	qemu-system-x86_64 -M q35 -m 2G -hda $(IMAGE_NAME).hdd -boot c

run-kvm: all
	qemu-system-x86_64 -M q35 -m 2G -hda $(IMAGE_NAME).hdd -boot c --enable-kvm

.PHONY: run-hdd-uefi
run-uefi: ovmf install_hdd
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf/OVMF.fd -hda $(IMAGE_NAME).hdd

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v5.x-branch-binary --depth=1
	$(MAKE) -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

$(IMAGE_NAME).hdd:
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	sgdisk $(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
	./limine/limine bios-install $(IMAGE_NAME).hdd

install_hdd: $(IMAGE_NAME).hdd limine kernel
	mformat -i $(IMAGE_NAME).hdd@@1M
	mmd -i $(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M kernel/bin/kernel.elf limine.cfg limine/limine-bios.sys ::/
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT

.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).hdd
	$(MAKE) -C kernel clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf
	$(MAKE) -C kernel distclean