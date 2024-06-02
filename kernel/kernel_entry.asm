[bits 32]
[extern kernel_main] ; Define calling point. Must have same name as kernel.c 'main' function

global __start
__start:
    mov eax, PAGE_DIR
    call fill_page_dir

    mov eax, PAGE_TABLE
    call fill_page_table

    mov eax, PAGE_TABLE
    ; or eax, 0x83  ; 4 mb
    or eax, 0x13  ; 4 kb
    mov [PAGE_DIR], eax

    call __enable_paging

    call kernel_main ; Calls the C function. The linker will know where it is placed in memory

halt:
    cli
    hlt
    jmp halt

; TODO functions to load paging table

PAGE_DIR equ 0x1000
PAGE_TABLE equ 0x2000

; eax = page dir 4 kb aligned
global __enable_paging
__enable_paging:
    pusha

    mov eax, PAGE_DIR
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    popa
    ret

; eax = page dir 4 kb aligned
fill_page_dir:
    pusha

    ; ecx = index
    xor ecx, ecx

    ; edx = value
    mov edx, 0x2

.loop:
    cmp ecx, 1024
    je .done

    mov [eax], edx
    add eax, 4
    inc ecx

    jmp .loop

.done:
    popa
    ret

; eax = page dir 4 kb aligned
fill_page_table:
    pusha

    ; ecx = index
    xor ecx, ecx

.loop:
    cmp ecx, 1024
    je .done

    mov edx, ecx
    shl edx, 12
    or edx, 3

    mov [eax], edx
    add eax, 4
    inc ecx

    jmp .loop

.done:
    popa
    ret
