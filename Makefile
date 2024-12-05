
# ========
#  CONFIG
# ========
CROSS_PREFIX ?= "${HOME}"/.local/opt/cross
GDB = $(CROSS_PREFIX)/bin/i386-elf-gdb
QEMU = qemu-system-i386 -boot order=a

# QEMUFLAGS = -m 4G -drive format=qcow2,file=drive.img -d int,cpu_reset -D qemu_log.txt -no-reboot
# QEMUFLAGS = -m 1G -drive format=raw,file=drive.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown
QEMUFLAGS = -m 1G -drive format=raw,file=build/apps.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown

# ===============
#  LAUNCH & UTIL
# ===============
setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build:
	cmake --build build

run:
	qemu-system-i386 -boot order=a -m 1G -drive format=raw,file=drive.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown -drive format=raw,file=build/os-image.bin,index=0,if=floppy

run_tty:
	qemu-system-i386 -boot order=a -m 1G -display curses -drive format=raw,file=drive.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown -drive format=raw,file=build/os-image.bin,index=0,if=floppy

debug:
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=build/os-image.bin,index=0,if=floppy
#	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf" -ex "b *0x7c00" -ex "b *0x7e00" -ex "b __start" -ex "b kernel_main" -ex "b kernel/src/cpu/isr.c:124"
#	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf" -ex "b kernel_main" -ex "b kernel/src/cpu/isr.c:124"

boot-debug:
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=build/os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7c00" -ex "b *0x7e00" -ex "b *0x8000"

dump:
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=build/os-image-dump.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_boot.bin 0x0 0x21000" -ex "kill" -ex "quit"

dump-kernel:
	$(QEMU) -s -S $(QEMUFLAGS) -drive format=raw,file=build/os-image.bin,index=0,if=floppy &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_kernel.bin 0x0 0x19000" -ex "kill" -ex "quit"

clean:
	rm -rf *.bin qemu_log.txt drive.img build/

.PHONY: setup build run debug boot-debug dump dump-kernel clean
