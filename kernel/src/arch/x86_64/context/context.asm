bits 64

section .text

global arch_get_context

arch_get_context:
    mov rax, gs:0x8
    ret