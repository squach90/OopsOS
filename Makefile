# Cross-compiler
AS=i686-elf-as
CC=i686-elf-gcc

# Directories
BUILD_DIR=build
KERNEL_DIR=kernel

# --- CHANGES START HERE ---
# 1. Automatically find all .c files in the kernel directory
C_SOURCES=$(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(KERNEL_DIR)/commands/*.c)

# 2. Convert that list of .c files into a list of .o files in the build directory
# Example: kernel/kernel.c -> build/kernel.o, kernel/libc.c -> build/libc.o
OBJS=$(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
OBJS += $(BUILD_DIR)/boot.o
# --- CHANGES END HERE ---

# Output binary and ISO
KERNEL_BIN=oopsos.bin
ISO=OopsOs.iso

# Flags
CFLAGS=-ffreestanding -m32 -O2 -Wall -Wextra -nostdlib
LDFLAGS=-T linker.ld -ffreestanding -m32 -nostdlib

all: $(ISO)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble bootloader
$(BUILD_DIR)/boot.o: boot.s | $(BUILD_DIR)
	$(AS) boot.s -o $(BUILD_DIR)/boot.o

# --- NEW PATTERN RULE ---
# This compiles ANY .c file in kernel/ into a .o file in build/
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel + bootloader (now uses the $(OBJS) list)
$(KERNEL_BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)
	@echo "Checking multiboot..."
	@if grub-file --is-x86-multiboot $(KERNEL_BIN) || grub2-file --is-x86-multiboot $(KERNEL_BIN); then \
		echo "Multiboot confirmed"; \
	else \
		echo "ERROR: The file is not multiboot"; exit 1; \
	fi

$(ISO): $(KERNEL_BIN)
	@mkdir -p iso/boot/grub
	@cp $(KERNEL_BIN) iso/boot/
	@echo 'menuentry "OopsOs" { multiboot /boot/oopsos.bin }' > iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso/ || grub2-mkrescue -o $(ISO) iso/

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 1024 -boot d -vga std

clean:
	rm -rf $(BUILD_DIR) $(KERNEL_BIN) $(ISO) iso