
# ========
#  CONFIG
# ========
ASM:=nasm
CC:=i386-elf-gcc
LD:=i386-elf-ld
GDB:=gdb
QEMU:=qemu-system-i386

CFLAGS := -g -Werror -ffreestanding

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

os-image-dump.bin: $(PWD)$(OBJDIR)/bootsect.bin $(PWD)$(OBJDIR)/second.bin
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

$(PWD)$(OBJDIR)/zero:
	dd if=/dev/zero of=$@ bs=512 count=2048

# ========
#  KERNEL
# ========
$(PWD)$(OBJDIR)/kernel.bin: $(OBJ)
	$(LD) -Ttext 0x8000 --oformat binary -o $@ $^

$(PWD)$(OBJDIR)/kernel.elf: ${OBJ}
	$(LD) -Ttext 0x8000 -o $@ $^

$(PWD)$(OBJDIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(PWD)$(OBJDIR)/%.o: %.asm
	@mkdir -p $(shell dirname $@)
	$(ASM) -f elf $< -o $@

# ===============
#  LAUNCH & UTIL
# ===============
run: os-image.bin drive.img
	$(QEMU) -drive format=raw,file=os-image.bin,index=0,if=floppy -drive format=qcow2,file=drive.img

debug: os-image.bin $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -S -drive format=raw,file=os-image.bin,index=0,if=floppy -drive format=qcow2,file=drive.img &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(PWD)$(OBJDIR)/kernel.elf" -ex "b *0x7c00" -ex "b *0x8000" -ex "b __start" -ex "b kernel_main"

dump: os-image-dump.bin $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -S -drive format=raw,file=os-image-dump.bin,index=0,if=floppy -drive format=qcow2,file=drive.img &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x8000" -ex "c" -ex "dump binary memory second.bin 0x8000 0x18000" -ex "kill" -ex "quit"

debugbuild:
	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"
	@echo -e "OBJ\n$(OBJ)\n"

clean:
	rm -rf os-image.bin os-image-dump.bin $(PWD)$(OBJDIR)

.PHONY: all run debug debugbuild clean
