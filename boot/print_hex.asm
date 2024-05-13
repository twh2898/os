print_hex:
    pusha
    
    mov cx, 0

.print_hex_loop:
    cmp cx, 4
    je .print_hex_stop

    mov ax, dx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39
    jle .print_hex_number
    add al, 7

.print_hex_number:
    mov bx, HEX_OUT + 5
    sub bx, cx
    mov [bx], al
    ror dx, 4

    add cx, 1
    jmp .print_hex_loop

.print_hex_stop:
    mov bx, HEX_OUT
    call print
    popa
    ret

HEX_OUT:
    db "0x0000", 0
