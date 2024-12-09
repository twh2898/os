[bits 32]

global send_interrupt

send_interrupt:
    mov eax, [esp+4]
    mov ebx, [esp+8]
    mov ecx, [esp+12]

    int 48

    ret
