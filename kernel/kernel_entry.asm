[bits 32]

section .multiboot
    align 4
    dd 0x1BADB002   ; magic
    dd 0x00         ; flags
    dd - 0x1BADB002 ; checksum

section .text
extern kernel_main
global __start
__start:
    call kernel_main

halt:
    hlt
    jmp halt
