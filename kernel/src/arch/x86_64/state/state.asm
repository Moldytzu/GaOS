bits 64

section .text

%macro PUSH_REG 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    mov rax, cr3
    push rax
%endmacro

%macro POP_REG 0
    pop rax
    mov cr3, rax
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

global arch_switch_state

; switches to a new state 
; rdi=switch stack
arch_switch_state:
    mov rsp, rdi
    POP_REG      ; restore GPRs
    add rsp, 8   ; skip error field
    iretq        ; restore flags, segments, stack and instruction pointer