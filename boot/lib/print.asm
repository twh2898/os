print:
    pusha

; keep this in mind:
; while (string[i] != 0) { print string[i]; i++ }

; the comparison for string end (null byte)
.start:
    mov al, [bx] ; 'bx' is the base address for the string
    cmp al, 0 
    je .done

    ; the part where we print with the BIOS help
    mov ah, 0x0e
    int 0x10 ; 'al' already contains the char

    ; increment pointer and do next loop
    add bx, 1
    jmp .start

.done:
    popa
    ret

print_nl:
    pusha
    
    mov ah, 0x0e
    mov al, 0x0a ; newline char
    int 0x10
    mov al, 0x0d ; carriage return
    int 0x10
    
    popa
    ret

print_hex:
    pusha
    
    xor cx, cx

.loop:
    cmp cx, 4
    je .stop

    mov ax, dx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39
    jle .number
    add al, 7

.number:
    mov bx, .HEX_OUT + 5
    sub bx, cx
    mov [bx], al
    ror dx, 4

    add cx, 1
    jmp .loop

.stop:
    mov bx, .HEX_OUT
    call print
    popa
    ret

.HEX_OUT:
    db "0x0000", 0
