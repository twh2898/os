
# ========
#  CONFIG
# ========
ASM:=nasm
CC:="${HOME}/.local/opt/cross/bin/i386-elf-gcc"
LD:="${HOME}/.local/opt/cross/bin/i386-elf-ld"
GDB:="${HOME}/.local/opt/cross/bin/i386-elf-gdb"
QEMU:=qemu-system-i386

CFLAGS := -g -Werror -ffreestanding
# CXXFLAGS := -fno-exceptions -fno-rtti
LDFLAGS := -nostdlib -L"${HOME}/.local/opt/cross/lib/gcc/i386-elf/12.2.0" -lgcc
QEMUFLAGS := -cpu host -enable-kvm -m 4G -drive format=qcow2,file=drive.img -d int,cpu_reset -D qemu_log.txt -machine smm=off -no-reboot

SRCDIR:=kernel/src
INCDIR:=kernel/include
BOOTDIR:=boot
OBJDIR:=build

# =======
#  BUILD
# =======
PWD :=
PWD := $(shell pwd)/
CFLAGS += -I$(PWD)$(INCDIR)

BOOT_SOURCES := $(shell find "$(BOOTDIR)" -name '*.asm')
C_SOURCES := $(shell find "$(SRCDIR)" -name '*.c')
ASM_SOURCES := $(shell find "$(SRCDIR)" -name '*.asm')
HEADERS := $(shell find "$(INCDIR)" -name '*.h')

OBJ := $(OBJDIR)/kernel/kernel_entry.o ${C_SOURCES:%.c=$(OBJDIR)/%.o} ${ASM_SOURCES:%.asm=$(OBJDIR)/%.o}

C_SOURCES := $(addprefix $(PWD), $(C_SOURCES))
HEADERS := $(addprefix $(PWD), $(HEADERS))
OBJ := $(addprefix $(PWD), $(OBJ))

# ==========
#  OS IMAGE
# ==========
os-image.bin: $(PWD)$(OBJDIR)/bootsect.bin $(PWD)$(OBJDIR)/kernel.bin
	cat $^ > $@
	@echo "Final image size"
	@du -sh $@
	@echo Remember the limit is 64K? in bootsect.asm

os-image-dump.bin: $(PWD)$(OBJDIR)/bootsect.bin $(PWD)$(OBJDIR)/second.bin $(PWD)$(OBJDIR)/third.bin
	cat $^ > $@

drive.img:
	qemu-img create -f qcow2 drive.img 100M

# ==========
#  BOOTSECT
# ==========
$(PWD)$(OBJDIR)/bootsect.bin: $(PWD)$(BOOTDIR)/bootsect.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(PWD)$(OBJDIR)/second.bin: $(PWD)$(BOOTDIR)/second.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(PWD)$(OBJDIR)/third.bin: $(PWD)$(BOOTDIR)/third.asm $(BOOT_SOURCES)
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

$(PWD)$(OBJDIR)/zero:
	dd if=/dev/zero of=$@ bs=512 count=2048

# ========
#  KERNEL
# ========
$(PWD)$(OBJDIR)/kernel.bin: Makefile link.ld $(OBJ)
	$(LD) -Tlink.ld --oformat binary -o $@ $(OBJ) $(LDFLAGS)

$(PWD)$(OBJDIR)/kernel.elf: Makefile $(OBJ)
	$(LD) -Ttext 0x8000 -o $@ $(OBJ) $(LDFLAGS)

$(PWD)$(OBJDIR)/%.o: %.c $(HEADERS) Makefile
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(PWD)$(OBJDIR)/%.o: %.asm Makefile
	@mkdir -p $(shell dirname $@)
	$(ASM) -f elf $< -o $@

# ===============
#  LAUNCH & UTIL
# ===============
run: os-image.bin drive.img
	$(QEMU) $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy

debug: os-image.bin $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(PWD)$(OBJDIR)/kernel.elf" -ex "b *0x7c00" -ex "b *0x8000" -ex "b __start" -ex "b kernel_main"

boot-debug: os-image-dump.bin
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7c00" -ex "b *0x8000"

dump: os-image-dump.bin $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image-dump.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x8000" -ex "c" -ex "dump binary memory dump_boot.bin 0x8000 0x19000" -ex "kill" -ex "quit"

dump-kernel: os-image.bin $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x8000" -ex "c" -ex "dump binary memory dump_kernel.bin 0x8000 0x19000" -ex "kill" -ex "quit"

debugbuild:
	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"
	@echo -e "OBJ\n$(OBJ)\n"

clean:
	rm -rf os-image.bin os-image-dump.bin $(PWD)$(OBJDIR)

.PHONY: all run debug debugbuild clean
