[org 0x7c00]
DATA equ 0x500
STACK equ 0x7bff
SECOND_STAGE equ 0x7e00
KERNEL_OFFSET equ 0x8000

mov [BOOT_DRIVE], dl
mov bp, STACK
mov sp, bp

call detect_mem

call load_kernel
call switch_to_pm
; jmp SECOND_STAGE
halt:
    hlt
    jmp halt

%include "boot/lib/print.asm"
%include "boot/lib/memory.asm"
%include "boot/lib/disk.asm"
%include "boot/lib/gdt.asm"
%include "boot/lib/switch.asm"

[bits 32]
BEGIN_PM:
    call SECOND_STAGE

halt_32:
    hlt
    jmp halt_32

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
