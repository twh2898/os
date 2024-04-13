
ASM=nasm
CC=i386-elf-gcc
LD=i386-elf-ld
ASMFLAGS=-f bin
CCFLAGS=-ffreestanding
LDFLAGS=-Ttext 0x0 --oformat binary

run: os-image.bin
	qemu-system-x86_64 -fda $<

os-image.bin: boot_sect_main.bin kernel.bin
	cat $^ > $@

kernel.bin: kernel_entry.o kernel.o
	$(LD) -Ttext 0x1000 --oformat binary -o $@ $^

kernel_entry.o: kernel_entry.asm
	$(ASM) -f elf $< -o $@

%.bin : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.bin : %.o
	$(LD) $(LDFLAGS) -o $@ $<

clean:
	@rm -rf *.bin *.o

.PHONY: clean
