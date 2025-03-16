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
