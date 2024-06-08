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

    call kernel_main ; Calls the C function. The linker will know where it is placed in memory

halt:
    cli
    hlt
    jmp halt

; TODO functions to load paging table

PAGE_DIR equ 0x1000
PAGE_TABLE equ 0x2000

; eax = page dir 4 kb aligned
global mmu_enable_paging
mmu_enable_paging:
    pusha

    mov eax, PAGE_DIR
    call mmu_change_dir

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    popa
    ret

global mmu_disable_paging
mmu_disable_paging:
    pusha

    mov eax, cr0
    and eax, 0x7fffffff
    mov cr0, eax

    popa
    ret

global mmu_paging_enabled
mmu_paging_enabled:
    pusha

    mov eax, cr0
    shr eax, 31
    and eax, 1

    popa
    ret

; void mmu_change_dir(mmu_page_dir_t * dir)
global mmu_change_dir
mmu_change_dir:
    shr eax, 12
    shl eax, 12
    mov cr3, eax
    ret

; mmu_page_dir_t * mmu_get_curr_dir()
global mmu_get_curr_dir
mmu_get_curr_dir:
    mov eax, cr3
    shr eax, 12
    shl eax, 12
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
