
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

# ========
#  OS Image
# ========
os.iso: $(PWD)isofiles/boot/kernel.bin $(PWD)isofiles/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(PWD)isofiles

# ========
#  KERNEL
# ========
$(PWD)isofiles/boot/kernel.bin: $(PWD)$(OBJDIR)/kernel.elf
	cp $< $@

$(PWD)$(OBJDIR)/kernel.elf: link.ld $(OBJ)
	$(LD) -T link.ld -o $@ $(OBJ)

$(PWD)$(OBJDIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(PWD)$(OBJDIR)/%.o: %.asm
	@mkdir -p $(shell dirname $@)
	$(ASM) -f elf $< -o $@

# ===============
#  LAUNCH & UTIL
# ===============
run: os.iso drive.img
	$(QEMU) -cdrom $< -drive format=qcow2,file=drive.img

debug: $(PWD)$(OBJDIR)/kernel.elf
	$(QEMU) -s -kernel $< -drive format=qcow2,file=drive.img &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $<" -ex "b __start" -ex "b kernel_main"

debugbuild:
	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"
	@echo -e "OBJ\n$(OBJ)\n"

drive.img:
	qemu-img create -f qcow2 drive.img 100M

clean:
	rm -rf os.iso drive.img isofiles/boot/kernel.bin os-image.bin kernel.bin $(PWD)$(OBJDIR)

.PHONY: all run debug debugbuild clean
