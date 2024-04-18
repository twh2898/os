
ASM=nasm
CC=i386-elf-gcc
LD=i386-elf-ld
GDB=gdb
QEMU=qemu-system-i386

CFLAGS=-g -Werror -I.

C_SOURCES = $(wildcard kernel/*.c drivers/*.c libc/*.c libc/intern/*.c cpu/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h libc/*.h libc/intern/*.h cpu/*.h)

OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > $@
	@echo "Final image size"
	@du -sh $@
	@echo Remember the limit is 32K in bootsect.asm

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext 0x1000 --oformat binary -o $@ $^

kernel.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext 0x1000 -o $@ $^

run: os-image.bin
	$(QEMU) -fda os-image.bin

debug: os-image.bin kernel.elf
	$(QEMU) -s -fda os-image.bin &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf" -ex "b __start" -ex "b kernel_main"

%.o : %.c ${HEADERS}
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o : %.asm
	$(ASM) -f elf $< -o $@

%.bin : %.asm
	$(ASM) -f bin $< -o $@

clean:
	@rm -rf */*.bin */*.o *.bin *.elf libc/intern/*.o

.PHONY: run debug clean
