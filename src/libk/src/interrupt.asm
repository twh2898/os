[bits 32]

global send_interrupt_noret
global send_interrupt

send_interrupt_noret:
send_interrupt:
    mov eax, [esp+4]
    mov ebx, esp
    add ebx, 8

    int 48

    ret
