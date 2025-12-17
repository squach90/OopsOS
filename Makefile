# Cross-compiler
AS=i686-elf-as
CC=i686-elf-gcc
LD=i686-elf-ld

# Sources
BOOT_SRC=boot.s
KERNEL_SRC=kernel/kernel.c

# Build directory for object files
BUILD_DIR=build
BOOT_OBJ=$(BUILD_DIR)/boot.o
KERNEL_OBJ=$(BUILD_DIR)/kernel.o

# Output binary and ISO
KERNEL_BIN=oopsos.bin
ISO=OopsOs.iso

# Flags
CFLAGS=-ffreestanding -m32 -O2 -Wall -Wextra -nostdlib
LDFLAGS=-T linker.ld -ffreestanding -m32 -nostdlib

# Default target
all: $(ISO)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble bootloader
$(BOOT_OBJ): $(BOOT_SRC) | $(BUILD_DIR)
	$(AS) $(BOOT_SRC) -o $(BOOT_OBJ)

# Compile kernel
$(KERNEL_OBJ): $(KERNEL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(KERNEL_SRC) -o $(KERNEL_OBJ)

# Link kernel + bootloader
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ)
	$(CC) $(LDFLAGS) -o $(KERNEL_BIN) $(BOOT_OBJ) $(KERNEL_OBJ)
	@echo "Checking multiboot..."
	@if grub2-file --is-x86-multiboot $(KERNEL_BIN); then \
		echo "Multiboot confirmed"; \
	else \
		echo "ERROR: The file is not multiboot"; exit 1; \
	fi

# Build ISO
$(ISO): $(KERNEL_BIN)
	@mkdir -p iso/boot
	@cp $(KERNEL_BIN) iso/boot/
	@echo "Creating grub.cfg..."
	@mkdir -p iso/boot/grub
	@echo 'menuentry "OopsOs" { multiboot /boot/oopsos.bin }' > iso/boot/grub/grub.cfg
	grub2-mkrescue -o $(ISO) iso/

# Run in QEMU
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 1024 -boot d -vga std

# Clean
clean:
	rm -rf $(BUILD_DIR) $(KERNEL_BIN) $(ISO) iso
