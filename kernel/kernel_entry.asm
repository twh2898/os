[bits 32]

section .multiboot
header_start:
    dd 0xe85250d6                ; magic number (multiboot 2)
    dd 0                         ; architecture 0 (protected mode i386)
    dd header_end - header_start ; header length
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; insert optional multiboot tags here

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

section .text
extern kernel_main
global __start
__start:
    call ok
    call kernel_main

halt:
    hlt
    jmp halt

; print `OK` to screen
ok:
    mov dword [0xb8000], 0x2f4b2f4f
    ret

global MULTIBOOT_SECTOR
MULTIBOOT_SECTOR times 4 db 0
