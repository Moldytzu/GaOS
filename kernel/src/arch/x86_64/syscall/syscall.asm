bits 64

section .text

global arch_syscall_entry
extern arch_syscall_handler

arch_syscall_entry:
    ; first, swap the stack to a known one

    mov r10, gs:0x0 ; read context type
    cmp r10, 0      ; if it equals to cpu
    je .swap_stack  ; swap the stack directly
    swapgs          ; else, swap the context first

.swap_stack:
    mov gs:0x10, rsp ; save the userspace stack
    mov rsp, gs:0x8  ; load our new stack

    ; r11 holds old rflags
    ; rcx holds old rip
    push r11
    push rcx

    mov rcx, rax
    call arch_syscall_handler

    pop rcx
    pop r11

    mov rsp, gs:0x10 ; restore the userspace stack

    o64 sysret