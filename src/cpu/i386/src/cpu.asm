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

; https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial

;C declaration:
;   void switch_to_task(thread_control_block *next_thread);
;
;WARNING: Caller is expected to disable IRQs before calling, and enable IRQs again after function returns

TCB_EIP equ 4
TCB_ESP equ 8
TCB_CR3 equ 12
TCB_ESP0 equ 16
TSS_ESP0 equ 20

current_task_TCB: dd 0

global set_first_task
set_first_task:
    mov eax, [esp + 4]
    mov [current_task_TCB], eax
    ret

global switch_to_task
switch_to_task:

    ;Save previous task's state

    ;Notes:
    ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    ;  EIP is already saved on the stack by the caller's "CALL" instruction
    ;  The task isn't able to change CR3 so it doesn't need to be saved
    ;  Segment registers are constants (while running kernel code) so they don't need to be saved

    push ebx
    push esi
    push edi
    push ebp

    mov edi,[current_task_TCB]    ;edi = address of the previous task's "thread control block"
    mov [edi+TCB_ESP],esp         ;Save ESP for previous task's kernel stack in the thread's TCB

    ;Load next task's state

    mov esi,[esp+(4+1)*4]         ;esi = address of the next task's "thread control block" (parameter passed on stack)
    mov [current_task_TCB],esi    ;Current task's TCB is the next task TCB

    mov esp,[esi+TCB_ESP]         ;Load ESP for next task's kernel stack from the thread's TCB
    mov eax,[esi+TCB_CR3]         ;eax = address of page directory for next task
    mov ebx,[esi+TCB_ESP0]        ;ebx = address for the top of the next task's kernel stack
    mov [TSS_ESP0],ebx            ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov ecx,cr3                   ;ecx = previous task's virtual address space

    cmp eax,ecx                   ;Does the virtual address space need to being changed?
    je .doneVAS                   ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov cr3,eax                   ; yes, load the next task's virtual address space

.doneVAS:

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret                           ;Load next task's EIP from its kernel stack