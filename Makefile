
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
	@echo Remember the limit is 32K? in bootsect.asm

drive.img:
	qemu-img create -f qcow2 drive.img 100M

# ==========
#  BOOTSECT
# ==========
$(PWD)$(OBJDIR)/bootsect.bin: $(PWD)$(BOOTDIR)/bootsect.asm
	@mkdir -p $(shell dirname $@)
	$(ASM) -f bin $< -o $@

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
	$(QEMU) -s -drive format=raw,file=os-image.bin,index=0,if=floppy -drive format=qcow2,file=drive.img &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(PWD)$(OBJDIR)/kernel.elf" -ex "b __start" -ex "b kernel_main"

debugbuild:
	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"
	@echo -e "OBJ\n$(OBJ)\n"

clean:
	rm -rf os-image.bin $(PWD)$(OBJDIR)

.PHONY: all run debug debugbuild clean
