[org 0x7c00]
KERNEL_OFFSET equ 0x8000
KERNEL_OFFSET2 equ 0x10000

mov [BOOT_DRIVE], dl
mov bp, 0x7bff
mov sp, bp

call detect_mem

call load_kernel
call switch_to_pm
; jmp KERNEL_OFFSET
halt:
    hlt
    jmp halt

%include "boot/print.asm"
%include "boot/print_hex.asm"
%include "boot/disk.asm"
%include "boot/memory.asm"
%include "boot/gdt.asm"
%include "boot/switch.asm"

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET

halt_32:
    hlt
    jmp halt_32

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
