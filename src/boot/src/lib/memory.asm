DATA_AREA_LOW equ DATA
DATA_AREA_COUNT equ DATA + 2
DATA_AREA_HIGH equ DATA + 4

detect_mem:
    pusha

.lower:
    clc
    int 0x12
    jc .error

    mov [DATA_AREA_LOW], ax

    xor ebx, ebx
    mov [DATA_AREA_COUNT], bx
    mov es, ebx
    mov di, DATA_AREA_HIGH

.upper:
    mov edx, 0x534D4150
    mov ecx, 24
    xor eax, eax
    mov eax, 0xe820
    int 0x15

    jc .error

    cmp eax, 0x534D4150
    jne .error

    push ebx

    mov bx, [DATA_AREA_COUNT]
    inc bx
    mov [DATA_AREA_COUNT], bx
    
    pop ebx
    cmp ebx, 0
    je .done

    add di, 24
    jmp .upper

.done:

    popa
    ret

.error:
    mov bx, .MSG_MEM_ERROR
    call print
    call print_nl
    jmp halt

.MSG_MEM_ERROR:
    db "Failed to detect memory", 0
