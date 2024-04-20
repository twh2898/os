
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
# PWD := $(shell pwd)/
CFLAGS += -I$(PWD)$(INCDIR)

C_SOURCES := $(shell find "$(SRCDIR)" -name '*.c')
ASM_SOURCES := $(shell find "$(SRCDIR)" -name '*.asm')
HEADERS := $(shell find "$(INCDIR)" -name '*.h')

OBJ := $(OBJDIR)/kernel/kernel_entry.o ${C_SOURCES:%.c=$(OBJDIR)/%.o} ${ASM_SOURCES:%.asm=$(OBJDIR)/%.o}

C_SOURCES := $(addprefix $(PWD), $(C_SOURCES))
HEADERS := $(addprefix $(PWD), $(HEADERS))
OBJ := $(addprefix $(PWD), $(OBJ))

kernel.bin: $(OBJ)
	$(LD) -Ttext 0x1000 --oformat binary -o $@ $^

debugbuild:
	@echo -e "CFLAGS\n$(CFLAGS)\n"
	@echo -e "C_SOURCES\n$(C_SOURCES)\n"
	@echo -e "HEADERS\n$(HEADERS)\n"
	@echo -e "OBJ\n$(OBJ)\n"

$(PWD)$(OBJDIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(PWD)$(OBJDIR)/%.o: %.asm
	@mkdir -p $(shell dirname $@)
	$(ASM) -f elf $< -o $@

clean:
	rm -rf kernel.bin $(PWD)$(OBJDIR)

.PHONY: all clean run debug debugbuild
