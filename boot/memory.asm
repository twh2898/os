DATA_AREA equ 0x7E00
DATA_AREA_LOW equ DATA_AREA
DATA_AREA_COUNT equ DATA_AREA+2
DATA_AREA_HIGH equ DATA_AREA+4

detect_mem:
    pusha

.mem_lower:
    clc
    int 0x12
    jc .mem_error

    mov [DATA_AREA_LOW], ax

    xor ebx, ebx
    mov [DATA_AREA_COUNT], bx
    mov es, ebx
    mov di, DATA_AREA_HIGH

.mem_upper:
    mov edx, 0x534D4150
    mov ecx, 24
    xor eax, eax
    mov eax, 0xe820
    int 0x15

    jc .mem_error

    cmp eax, 0x534D4150
    jne .mem_error

    push ebx

    mov bx, [DATA_AREA_COUNT]
    inc bx
    mov [DATA_AREA_COUNT], bx
    
    pop ebx
    cmp ebx, 0
    je .mem_done

    add di, 24
    jmp .mem_upper

.mem_done:

    popa
    ret

.mem_error:
    mov bx, MSG_MEM_ERROR
    call print
    call print_nl
    jmp halt

MSG_MEM_ERROR:
    db "Failed to detect memory", 0
