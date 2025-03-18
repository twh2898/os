[bits 32]
[extern kernel_main] ; Define calling point. Must have same name as kernel.c 'main' function

; void __start(void);
global __start
__start:
    call kernel_main ; Calls the C function. The linker will know where it is placed in memory

; If the kernel ever returns, halt
; _Noreturn void halt(void);
global halt
halt:
    cli
    hlt
    jmp halt

[extern register_kernel_exit]

; void jump_kernel_mode(void * fn);
global jump_kernel_mode
jump_kernel_mode:
    mov eax,   [esp+4]
    mov [.cb], eax

    mov  eax, .exit
    push eax

    mov  eax, cr3
    push eax

    push ebp

    push esp

    call register_kernel_exit

.exit:
    call [.cb]

    ret

.cb: dd 0

; void jump_proc(uint32_t cr3, uint32_t esp, uint32_t call);
global jump_proc
jump_proc:
    mov eax, [esp+4]  ; cr3
    mov ebx, [esp+8]  ; esp
    mov edx, [esp+12] ; call

    mov cr3, eax
    mov ebp, ebx
    mov esp, ebx

    jmp edx

global jump_usermode
jump_usermode:
    mov ebx, [esp+4]
    mov ax,  (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
    mov ds,  ax
    mov es,  ax
    mov fs,  ax
    mov gs,  ax          ; SS is handled by iret

    ; set up the stack frame iret expects
    mov   eax, esp
    push  (4 * 8) | 3 ; data selector
    push  eax         ; current esp
    pushf             ; eflags
    push  (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)
    push  ebx         ; instruction address to return to
    iret

; https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial

;C declaration:
;   void switch_to_task(thread_control_block *next_thread);
;
;WARNING: Caller is expected to disable IRQs before calling, and enable IRQs again after function returns

TCB_CR3           equ 4
TCB_ESP           equ 8
TCB_ESP0          equ 12

current_task_TCB: dd  0

; void start_task(uint32_t cr3, uint32_t esp, uint32_t eip, const ebus_event_t * event)
global start_task
start_task:
    mov ebp, esp
    mov ecx, [ebp+4]  ; cr3
    mov edx, [ebp+8]  ; esp
    mov ebx, [ebp+12] ; eip
    mov eax, [ebp+16] ; event

    mov esp, edx
    mov cr3, ecx

    jmp ebx


; void resume_task(uint32_t cr3, uint32_t esp, const ebus_event_t * event)
global resume_task
resume_task:
    mov ebp, esp
    mov ecx, [ebp+4]  ; cr3
    mov ebx, [ebp+8]  ; esp
    mov eax, [ebp+12] ; event

    ; TODO get tss and set it's esp0
    ; mov [TSS_ESP0], ebx ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov edi, cr3 ;edi = previous task's virtual address space

    cmp ecx, edi ;Does the virtual address space need to being changed?
    je  .done    ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov cr3, ecx ; yes, load the next task's virtual address space

.done:

    mov esp, ebx

    ret ;Load next task's EIP from its kernel stack

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

    mov edi,           [current_task_TCB] ;edi = address of the previous task's "thread control block"
    mov [edi+TCB_ESP], esp                ;Save ESP for previous task's kernel stack in the thread's TCB

    ;Load next task's state

    mov esi,                [esp+(4+1)*4] ;esi = address of the next task's "thread control block" (parameter passed on stack)
    mov [current_task_TCB], esi           ;Current task's TCB is the next task TCB

    mov esp, [esi+TCB_ESP]  ;Load ESP for next task's kernel stack from the thread's TCB
    mov eax, [esi+TCB_CR3]  ;eax = address of page directory for next task
    mov ebx, [esi+TCB_ESP0] ;ebx = address for the top of the next task's kernel stack
    ; mov [TSS_ESP0], ebx            ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov ecx, cr3            ;ecx = previous task's virtual address space

    cmp eax, ecx ;Does the virtual address space need to being changed?
    je  .doneVAS ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov cr3, eax ; yes, load the next task's virtual address space

.doneVAS:

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret ;Load next task's EIP from its kernel stack
