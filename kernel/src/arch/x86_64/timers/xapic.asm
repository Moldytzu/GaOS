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

extern arch_xapic_isr_handler
global arch_xapic_isr_handler_entry

arch_xapic_isr_handler_entry:
    cli                         ; disable interrupts
    sub rsp, 8                  ; fake an error code on stack
    PUSH_REG                    ; push GPRs
    mov rdi, rsp                ; point to them
    jmp arch_xapic_isr_handler  ; jump in the C handler