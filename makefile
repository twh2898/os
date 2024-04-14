C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
#C_SOURCES = $(wildcard kernel/*.c)
#HEADERS = $(wildcard kernel/*.h)

OBJ = ${C_SOURCES:.c=.o}

ASM=nasm
CC=i386-elf-gcc
LD=i386-elf-ld
GDB=i386-elf-gdb
QEMU=qemu-system-i386

CFLAGS=-g
LDFLAGS=-Ttext 0x0 --oformat binary

os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > $@

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext 0x1000 --oformat binary -o $@ $^

kernel.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext 0x1000 -o $@ $^

run: os-image.bin
	$(QEMU) -fda os-image.bin

debug: os-image.bin kernel.elf
	$(QEMU) -s -fda os-image.bin &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o : %.c ${HEADERS}
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o : %.asm
	$(ASM) -f elf $< -o $@

%.bin : %.asm
	$(ASM) -f bin $< -o $@

clean:
	@rm -rf **/*.bin **/*.o *.bin *.elf

.PHONY: clean
