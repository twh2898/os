[org 0x7c00]
KERNEL_OFFSET equ 0x8000
KERNEL_OFFSET2 equ 0x10000

mov [BOOT_DRIVE], dl
mov bp, 0x7bff
mov sp, bp

call load_kernel
call switch_to_pm
; jmp KERNEL_OFFSET
jmp $

%include "boot/print.asm"
%include "boot/print_hex.asm"
%include "boot/disk.asm"
%include "boot/gdt.asm"
%include "boot/switch.asm"

[bits 16]
load_kernel:
    mov eax, 512
    call disk_lba

    mov ebx, KERNEL_OFFSET >> 4
    mov es, ebx
    mov bx, 0
    mov al, 64  ; sector is 512 bytes, so 64 = 32K
    mov dl, [BOOT_DRIVE]
    call disk_load

    mov eax, 0x8000 ; = 64 * 512 = 32k
    call disk_lba

    mov ebx, KERNEL_OFFSET2 >> 4
    mov es, ebx
    mov bx, 0
    mov al, 64  ; sector is 512 bytes, so 64 = 32K
    mov dl, [BOOT_DRIVE]
    call disk_load

    mov ebx, 0
    mov es, ebx
    ret

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
