[bits 32]

gdtr dw 0 ; For limit storage
     dd 0 ; For base storage

global load_gdt
load_gdt:
    mov ax, [esp + 4]
    mov [gdtr], ax
    mov eax, [esp + 8]
    mov [gdtr + 2], eax
    lgdt [gdtr]
    call reloadSegments
    ret

reloadSegments:
   ; Reload CS register containing code selector:
   jmp   0x08:.reload_CS ; 0x08 is a stand-in for your code segment
.reload_CS:
   ; Reload data segment registers:
   mov   ax, 0x10 ; 0x10 is a stand-in for your data segment
   mov   ds, ax
   mov   es, ax
   mov   fs, ax
   mov   gs, ax
   mov   ss, ax
   ret

global flush_tss
flush_tss:
	mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
	ltr ax
	ret

global jump_usermode
jump_usermode:
    mov ebx, [esp+4]
	mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax ; SS is handled by iret

	; set up the stack frame iret expects
	mov eax, esp
	push (4 * 8) | 3 ; data selector
	push eax ; current esp
	pushf ; eflags
	push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)
	push ebx ; instruction address to return to
	iret

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
