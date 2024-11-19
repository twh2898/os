[org 0x8000]

jmp do_thing

db "Near", 0

%include "lib/print.asm"

do_thing:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_nl
    jmp do_other_thing

exit:
    jmp $

times (16*512)-($-$$) db 'b'
times (32*512)-($-$$) db 'C'
times (48*512)-($-$$) db 'd'
times (60*512)-($-$$) db 'E'
times (63*512)-($-$$) db 'f'

MSG_LOAD_KERNEL db "Far jump", 0

do_other_thing:
    mov bx, MSG_FAR_JUMP
    call print
    call print_nl
    jmp exit

MSG_FAR_JUMP db "yoo hoo, over here", 0

db 'bAb'
times (64*512)-($-$$)-5 db 'a'
db 0, 0, 'AbA'
; times 0x509 db 'A'
