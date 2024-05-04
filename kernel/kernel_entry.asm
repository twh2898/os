[bits 32]

section .multiboot
header_start:
    align 4
    dd 0x1BADB002
    dd 0x00
    dd -0x1BADB002

section .text
extern kernel_main
global __start
__start:
    cmp eax, 0x2BADB002
    jne halt

    mov [MULTIBOOT_SECTOR], ebx
    call kernel_main

halt:
    hlt
    jmp halt

global MULTIBOOT_SECTOR
MULTIBOOT_SECTOR times 4 db 0
