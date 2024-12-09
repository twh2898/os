[bits 32]

global send_interrupt

send_interrupt:
    mov eax, [esp+4]
    mov ebx, [esp+8]

    int 48

    ret
