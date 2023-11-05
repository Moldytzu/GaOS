bits 64

section .text

global arch_swap_stack

arch_swap_stack: ; rdi = new stack base, rsi = new stack size
    add rdi, rsi

    pop r11      ; pop return address
    mov rsp, rdi ; swap the stack
    push r11     ; push return address 

    ret