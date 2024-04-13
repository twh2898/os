
CC=i386-elf-gcc
LD=i386-elf-ld
CCFLAGS=-ffreestanding
LDFLAGS=-Ttext 0x0 --oformat binary

run: boot_sect_main

% : %.bin
	qemu-system-x86_64 $<

%.bin : %.asm
	nasm -f bin $< -o $@

%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.bin : %.o
	$(LD) $(LDFLAGS) -o $@ $<

clean:
	@rm -rf *.bin *.o

.PHONY: clean
