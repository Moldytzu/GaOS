bits 16

global arch_ap_entry16

arch_ap_entry16:

    ; write HI to COM1
    mov dx, 0x3F8
    mov al, 'H'
    out dx, al

    mov al, 'I'
    out dx, al
    
    jmp $