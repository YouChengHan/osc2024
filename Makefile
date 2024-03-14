TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy

SRC_DIR = src
BUILD_DIR = build

LINKER_FILE = $(SRC_DIR)/linker.ld
ENTRY = $(SRC_DIR)/start.s
ENTRY_OBJS = $(BUILD_DIR)/start.o
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(info SRCS: $(SRCS))
$(info OBJS: $(OBJS))

CFLAGS = -c -Wall -O2 -Iinclude -nostdinc -nostdlib -nostartfiles -ffreestanding -fno-stack-protector

.PHONY: all clean asm run debug dirs

all: dirs kernel8.img

dirs: 
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir "$(BUILD_DIR)"; fi

cpio:
	cd rootfs; find . | cpio -o -H newc > ../initramfs.cpio; cd ..
	
kernel8.img: $(OBJS) $(ENTRY_OBJS)
	$(LD) $(ENTRY_OBJS) $(OBJS) -T $(LINKER_FILE) -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

$(ENTRY_OBJS): $(ENTRY)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

asm: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm -initrd initramfs.cpio

run: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

display: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

debug: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

clean:
	rm -f $(BUILD_DIR)/* kernel8.*