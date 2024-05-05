disk_init:
    pusha

    ; reset drive
    ; mov ebx, 0
    ; mov es, ebx
    ; mov di, bx

    mov ah, 0x08
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error

    inc dh
    mov [FD_HPC], dh

    ; last index of sector per track
    mov dl, cl
    inc dl
    and dl, 0x1f
    mov [FD_SPT], dl

    ; last index of cylinder number
    ; mov dl, ch
    ; mov dh, cl
    ; shr dh, 5
    ; inc dx
    ; mov [FD_CYLS], dx

    popa
    ret

; lba in ax
disk_lba:
    mov bx, 512
    div bx
    mov [LBA], ax

    mov ax, ax
    mov bx, ax

    ;C = LBA / (HPC * SPT)

    ; ax = (HPC * SPT)
    mov al, [FD_SPT]
    mov bl, [FD_HPC]
    mul bx

    ; al = LBA / al
    mov bx, ax
    mov ax, [LBA]
    div bl
    mov [C], al

    ;H = (LBA / SPT) % HPC

    xor bx, bx

    ; al = LBA / SPT
    mov ax, [LBA]
    mov bl, [FD_SPT]
    div bl

    ; ah = al % HPC
    xor ah, ah
    mov bl, [FD_HPC]
    div bx
    mov [H], ah

    ;S = (LBA % SPT) + 1

    xor bx, bx
    mov ax, bx

    ; ah = LBA % SPT
    mov al, [LBA]
    mov bl, [FD_SPT]
    div bl
    inc ah
    mov [S], ah

    mov cl, [S]
    mov dh, [H]
    mov ch, [C]

    ret

disk_load:
    pusha

    push ax

    mov ah, 0x02
    int 0x13
    jc disk_error

    pop dx
    cmp al, dl
    jne sectors_error

    mov bx, DISK_FINISH
    call print

    mov dh, ah
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

LBA dw 0
FD_SPT db 0
FD_HPC db 0
; FD_CYLS dw 0
C db 0
H db 0
S db 0

DISK_FINISH:
    db "Done read ", 0

DISK_ERROR:
    db "Disk read error ", 0

SECTORS_ERROR:
    db "Incorrect number of secctors read ", 0
