bits 64

section .text

%include "registers.mac"

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