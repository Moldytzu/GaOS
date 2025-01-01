bits 64

section .text

global _syscall

_syscall:
    ; parameter   : num  1   2   3   4  5
    ; sysv abi    : rdi rsi rdx rcx r8 r9
    ; syscall abi : rdi rsi rdx r10 r8 r9
    mov r10, rcx
    syscall
    ret