[org 0x8000]

jmp do_thing

db "nEAR", 0

%include "lib/print.asm"

do_thing:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_nl
    jmp do_other_thing

exit:
    jmp $

times (16*512)-($-$$) db 'g'
times (32*512)-($-$$) db 'H'
times (48*512)-($-$$) db 'i'
times (60*512)-($-$$) db 'J'
times (63*512)-($-$$) db 'k'

MSG_LOAD_KERNEL db "Long jump", 0

do_other_thing:
    mov bx, MSG_FAR_JUMP
    call print
    call print_nl
    jmp exit

MSG_FAR_JUMP db "hoo yoo, here over", 0

db 'LmL'
times (64*512)-($-$$)-5 db 'L'
db 0, 0, 'MlM'
; times 0x509 db 'm'
