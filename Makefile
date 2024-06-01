
# ========
#  CONFIG
# ========
CROSS_PREFIX ?= "${HOME}"/.local/opt/cross
ASM = nasm
CC = $(CROSS_PREFIX)/bin/i386-elf-gcc
LD = $(CROSS_PREFIX)/bin/i386-elf-ld
GDB = $(CROSS_PREFIX)/bin/i386-elf-gdb
QEMU = qemu-system-i386

CFLAGS = -g -Werror -ffreestanding -I$(INCDIR)
CXXFLAGS = -fno-exceptions -fno-rtti
LDFLAGS = -nostdlib -L"$(CROSS_PREFIX)/lib/gcc/i386-elf/12.2.0" -lgcc
QEMUFLAGS = -m 4G -drive format=qcow2,file=drive.img -d int,cpu_reset -D qemu_log.txt -no-reboot

SRCDIR = kernel/src
INCDIR = kernel/include
BOOTDIR = boot
BUILD_DIR = build

# =======
#  BUILD
# =======

BOOT_SOURCES = $(shell find "$(BOOTDIR)" -name '*.asm')
C_SOURCES = $(shell find "$(SRCDIR)" -name '*.c')
ASM_SOURCES = $(shell find "$(SRCDIR)" -name '*.asm')
HEADERS = $(shell find "$(INCDIR)" -name '*.h')

OBJ = $(BUILD_DIR)/kernel/kernel_entry.o ${C_SOURCES:%.c=$(BUILD_DIR)/%.o} ${ASM_SOURCES:%.asm=$(BUILD_DIR)/%.o}

# ==========
#  OS IMAGE
# ==========
os-image.bin: $(BUILD_DIR)/bootsect.bin $(BUILD_DIR)/kernel.bin
	cat $^ > $@
	@echo "Final image size"
	@du -sh $@
	@echo Remember the limit is 96K in bootsect.asm

os-image-dump.bin: $(BUILD_DIR)/bootsect.bin $(BUILD_DIR)/second.bin $(BUILD_DIR)/third.bin
	cat $^ $(BUILD_DIR)/second.bin $(BUILD_DIR)/third.bin > $@

drive.img:
	qemu-img create -f qcow2 drive.img 100M

# ==========
#  BOOTSECT
# ==========
$(BUILD_DIR)/bootsect.bin: $(BOOTDIR)/bootsect.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/second.bin: $(BOOTDIR)/second.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/third.bin: $(BOOTDIR)/third.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/zero:
	dd if=/dev/zero of=$@ bs=512 count=2048

# ========
#  KERNEL
# ========
$(BUILD_DIR)/kernel.bin: Makefile link.ld $(OBJ)
	$(LD) -Tlink.ld --oformat binary -o $@ $(OBJ) $(LDFLAGS)

$(BUILD_DIR)/kernel.elf: Makefile $(OBJ)
	$(LD) -Tlink.ld -o $@ $(OBJ) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c $(HEADERS) Makefile
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm Makefile
	@mkdir -p $(shell dirname $@)
	$(ASM) -f elf $< -o $@

# ===============
#  LAUNCH & UTIL
# ===============
run: os-image.bin drive.img
	$(QEMU) $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy

debug: os-image.bin $(BUILD_DIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR)/kernel.elf" -ex "b *0x7c00" -ex "b *0x7e00" -ex "b __start" -ex "b kernel_main"

boot-debug: os-image-dump.bin
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7c00" -ex "b *0x7e00" -ex "b *0x8000"

dump: os-image-dump.bin $(BUILD_DIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image-dump.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_boot.bin 0x0 0x21000" -ex "kill" -ex "quit"

dump-kernel: os-image.bin $(BUILD_DIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_kernel.bin 0x0 0x19000" -ex "kill" -ex "quit"

debugbuild:
	@echo -e "CROSS_PREFIX\n$(CROSS_PREFIX)\n"
	@echo -e "ASM\n$(ASM)\n"
	@echo -e "CC\n$(CC)\n"
	@echo -e "LD\n$(LD)\n"
	@echo -e "GDB\n$(GDB)\n"
	@echo -e "QEMU\n$(QEMU)\n"

	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "CXXFLAGS\n$(CXXFLAGS)\n"
	@echo -e "LDFLAGS\n$(LDFLAGS)\n"
	@echo -e "QEMUFLAGS\n$(QEMUFLAGS)\n"

	@echo -e "BOOT_SOURCES\n$(BOOT_SOURCES)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "ASM_SOURCES\n$(ASM_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"

	@echo -e "OBJ\n$(OBJ)\n"

clean:
	rm -rf *.bin qemu_log.txt drive.img $(BUILD_DIR)

.PHONY: all run debug debugbuild clean
