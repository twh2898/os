
run: boot_sect_main

% : %.bin
	qemu-system-x86_64 $<

%.bin : %.asm
	nasm -f bin $< -o $@

clean:
	@rm -rf *.bin

.PHONY: clean
