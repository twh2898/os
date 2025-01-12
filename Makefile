
# ========
#  CONFIG
# ========
CROSS_PREFIX ?= "${HOME}"/.local/opt/cross
GDB = $(CROSS_PREFIX)/bin/i386-elf-gdb
# QEMU = qemu-system-i386 -cpu host -enable-kvm -boot order=a
# Using host cpu will disable debugger
QEMU = qemu-system-i386 -boot order=a

# QEMUFLAGS = -m 4G -drive format=qcow2,file=drive.img -d int,cpu_reset -D qemu_log.txt -no-reboot
# QEMUFLAGS = -m 1G -drive format=raw,file=drive.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown
QEMUFLAGS = -m 1G -drive format=raw,file=build/os-image.bin,index=0,if=floppy -drive format=raw,file=build/apps.tar -d int,mmu -D qemu_log.txt -no-reboot -no-shutdown

# ===============
#  LAUNCH & UTIL
# ===============
setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build:
	cmake --build build

run:
	$(QEMU) $(QEMUFLAGS)

run-tty:
	$(QEMU) -display curses $(QEMUFLAGS)

run-debug:
	$(QEMU) -s -S $(QEMUFLAGS)

debug:
	$(QEMU) -s -S $(QEMUFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/src/kernel/kernel.elf" -ex "b kernel_main" -ex "b isr_handler"

boot-debug:
	$(QEMU) -s -S $(QEMUFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7c00" -ex "b *0x7e00" -ex "b *0x8000"

dump:
	$(QEMU) -s -S $(QEMUFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_boot.bin 0x0 0x21000" -ex "kill" -ex "quit"

dump-kernel:
	$(QEMU) -s -S $(QEMUFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "b *0x7e00" -ex "c" -ex "dump binary memory dump_kernel.bin 0x0 0x19000" -ex "kill" -ex "quit"

test:
	make -C tests

test_cov:
	make -C tests_cov

lint:
	@find src tests/src -name '*.c' -or -name '*.h' -or -name '*.cpp' -or -name '*.hpp' | xargs clang-format --dry-run --Werror --sort-includes

format:
	@find src tests/src -name '*.c' -or -name '*.h' -or -name '*.cpp' -or -name '*.hpp' | xargs clang-format -i --Werror --sort-includes

clean:
	rm -rf *.bin qemu_log.txt drive.img build/

.PHONY: setup build run debug boot-debug dump dump-kernel test test_cov lint format clean
