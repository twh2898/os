[bits 32]

global apps_send_interrupt

apps_send_interrupt:
    int 48
    ret
