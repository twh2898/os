load_kernel:
    mov ebx, KERNEL_OFFSET >> 4
    mov es, ebx
    mov bx, 0
    mov al, 64  ; sector is 512 bytes, so 64 = 32K
    mov dl, [BOOT_DRIVE]
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    call disk_load

    mov ebx, KERNEL_OFFSET2 >> 4
    mov es, ebx
    mov bx, 0
    mov al, 64  ; sector is 512 bytes, so 64 = 32K
    mov dl, [BOOT_DRIVE]
    mov ch, 0x00
    mov dh, 0x01
    mov cl, 0x1e
    call disk_load

    mov ebx, 0
    mov es, ebx
    ret

disk_load:
    pusha

    push ax

    mov ah, 0x02
    int 0x13
    jc .disk_error

    pop dx
    cmp al, dl
    jne .sectors_error

    mov bx, DISK_FINISH
    call print

    mov dh, ah
    mov dl, al
    call print_hex
    call print_nl

    popa
    ret

.disk_error:
    mov bx, DISK_ERROR
    call print
    mov dh, ah
    call print_hex
    call print_nl
    jmp halt

.sectors_error:
    mov bx, SECTORS_ERROR
    call print
    mov dl, al
    mov dh, 0
    call print_hex
    call print_nl

DISK_FINISH:
    db "Done read ", 0

DISK_ERROR:
    db "Disk read error ", 0

SECTORS_ERROR:
    db "Incorrect number of secctors read ", 0
