[org 0x8000]

jmp do_thing

%include "boot/print.asm"
%include "boot/print_hex.asm"

do_thing:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_nl
    jmp do_other_thing

exit:
    jmp $

MSG_LOAD_KERNEL db "Far jump", 0

do_other_thing:
    mov bx, MSG_FAR_JUMP
    call print
    call print_nl
    jmp exit

MSG_FAR_JUMP db "yoo hoo, over here", 0
