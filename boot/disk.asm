disk_load:
    pusha

    push dx

    mov ah, 0x02
    mov al, dh
    mov cl, 0x02

    mov ch, 0x00
    mov dh, 0x00

    int 0x13
    jc disk_error

    pop dx
    cmp al, dh
    jne sectors_error
    mov bx, DISK_FINISH
    call print
    mov dh, 0
    mov dl, al
    call print_hex
    call print_nl
    popa
    ret

disk_error:
    mov bx, DISK_ERROR
    call print
    mov dh, ah
    call print_hex
    call print_nl
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call print
    mov dl, al
    mov dh, 0
    call print_hex
    call print_nl

disk_loop:
    jmp $

DISK_FINISH:
    db "Done read ", 0

DISK_ERROR:
    db "Disk read error ", 0

SECTORS_ERROR:
    db "Incorrect number of secctors read ", 0
