[bits 32]
[extern kernel_main] ; Define calling point. Must have same name as kernel.c 'main' function

global __start
__start:
    call kernel_main ; Calls the C function. The linker will know where it is placed in memory

halt:
    cli
    hlt
    jmp halt

; eax = page dir 4 kb aligned
global mmu_enable_paging
mmu_enable_paging:
    pusha

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
    mov eax, cr0
    shr eax, 31
    and eax, 1

    ret

; void mmu_change_dir(mmu_page_dir_t * dir)
global mmu_change_dir
mmu_change_dir:
    pusha

    shr eax, 12
    shl eax, 12
    mov cr3, eax

    popa
    ret

; mmu_page_dir_t * mmu_get_curr_dir()
global mmu_get_curr_dir
mmu_get_curr_dir:
    pusha

    mov eax, cr3
    shr eax, 12
    shl eax, 12

    popa
    ret
