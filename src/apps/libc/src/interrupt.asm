[bits 32]

global send_interrupt

send_interrupt:
    push eax
    push ebx

    mov eax, [esp+8]
    mov ebx, [esp+12]

    int 48

    pop ebx
    pop eax

    ret
