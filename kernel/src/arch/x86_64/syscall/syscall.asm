bits 64

section .text

global arch_syscall_entry
extern arch_syscall_handler

arch_syscall_entry:
    ; fixme: swap the stack

    ; r11 holds old rflags
    ; rcx holds old rip
    push r11
    push rcx

    mov rcx, rax
    call arch_syscall_handler

    pop rcx
    pop r11

    o64 sysret