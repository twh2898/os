print:
    pusha

; If current byte is 0, exit
; Else print
loop:
    mov al, [bx]
    cmp al, 0
    je zero

    mov ah, 0x0e
    int 0x10

    add bx, 1
    jmp loop

zero:
    popa
    ret

print_nl:
    pusha

    mov ah, 0x0e
    mov al, 0x0a  ; nl
    int 0x10
    mov al, 0x0d  ; cr
    int 0x10

    popa
    ret
