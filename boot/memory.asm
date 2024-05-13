detect_mem:
    pusha

.mem_lower:
    clc
    int 0x12
    jc .mem_error

    mov [DATA_AREA], ax

    mov dx, ax
    call print_hex
    call print_nl

.mem_upper:
    xor ebx, ebx
    mov es, ebx
    mov di, DATA_AREA+4
    xor ebx, ebx
    mov edx, 0x534D4150
    mov ecx, 24
    xor eax, eax
    mov eax, 0xe820
    int 0x15

    jc .mem_error

    cmp eax, 0x534D4150
    jne .mem_error

    popa
    ret

.mem_error:
    mov bx, MSG_MEM_ERROR
    call print
    call print_nl
    jmp halt

MSG_MEM_ERROR:
    db "Failed to detect memory", 0
