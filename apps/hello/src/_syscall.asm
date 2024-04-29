bits 64

section .text

global _syscall

_syscall:
    ; convert sysv abi to our abi
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rcx
    syscall
    ret