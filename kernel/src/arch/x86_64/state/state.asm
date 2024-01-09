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

global arch_switch_state, arch_save_simd_state, arch_restore_simd_state

; switches to a new state 
; rdi=switch stack
arch_switch_state:
    mov rsp, rdi
    POP_REG      ; restore GPRs
    add rsp, 8   ; skip error field
    iretq        ; restore flags, segments, stack and instruction pointer

; saves the current simd state in a given block of data
; rdi=block address
arch_save_simd_state:
    ; fixme: here we should use xsave if supported by the cpu
    ;        CPUID.01H:ECX.XSAVE[bit 26] and CR4.OSXSAVE[bit 18]

    o64 fxsave [rdi]
    ret

; restores the simd state from a given block of data
; rdi=block address
arch_restore_simd_state:
    ; fixme: here we should use xsave if supported by the cpu
    ;        CPUID.01H:ECX.XSAVE[bit 26] and CR4.OSXSAVE[bit 18]

    o64 fxrstor [rdi]
    ret