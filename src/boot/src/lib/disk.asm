load_kernel:
    pusha

    ; Use ebx to push es
    mov  ebx, es
    push ebx

    mov dl, [BOOT_DRIVE]

    mov ebx, SECOND_STAGE >> 4
    mov es,  ebx

    ; 0
    xor  bx, bx
    mov  al, 1     ; sector is 512 bytes, so 1 = 512
    mov  ch, 0x00
    mov  dh, 0x00
    mov  cl, 0x02
    call disk_load

    ; 512
    mov  bx, 0x200
    mov  al, 64    ; sector is 512 bytes, so 64 = 32K
    mov  cl, 0x03
    call disk_load

    ; 512 + 32K
    mov  bx, 0x8200
    mov  dh, 0x01
    mov  cl, 0x1f
    call disk_load

    mov ebx, (SECOND_STAGE + 0x8200 + 0x8000) >> 4
    mov es,  ebx
    xor ebx, ebx

    ; 512 + 64K
    dec  al
    mov  ch, 0x01
    mov  cl, 0x17
    call disk_load

    ; 96K

    ; Use ebx to pop es
    pop ebx
    mov es, ebx

    popa
    ret

disk_load:
    pusha

    push ax

    mov ah, 0x02
    int 0x13
    jc  .disk_error

    pop dx
    cmp al, dl
    jne .sectors_error

    popa
    ret

.disk_error:
    mov  bx, DISK_ERROR
    call print
    mov  dh, ah
    call print_hex
    call print_nl
    jmp  halt

.sectors_error:
    mov  bx, SECTORS_ERROR
    call print
    mov  dl, al
    mov  dh, 0
    call print_hex
    call print_nl

DISK_ERROR:
    db "DSKREADERROR", 0

SECTORS_ERROR:
    db "DSKSECTERROR ", 0
