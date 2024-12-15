bits 64

section .text

extern arch_isr_handlers, arch_isr_handler
global arch_idt_load, arch_interrupts_enabled

arch_idt_load: ; rdi = idtr
    lidt [rdi]
    ret

arch_interrupts_enabled:
    pushfq  ; push on stack the flags
    pop rax ; load them in rax
    
    and rax, 0x200 ; and with the interrupt enable flag mask
    shr rax, 9     ; shift right to have the result in the first bit
    ret 

; ISRs
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

; macro to generate handler
%macro GEN_HANDLER 1
global __isr_entry%1

__isr_entry%1:
%ifn %1 == 0x8 || %1 == 0xA || %1 == 0xB || %1 == 0xC || %1 == 0xD || %1 == 0xE || %1 == 0x11 || %1 == 0x15 || %1 == 0x1D || %1 == 0x1E
    sub rsp, 8 ; allocate an error code on the stack if the cpu doesn't do it for us 
%endif
    PUSH_REG              ; save old registers
    mov rdi, rsp          ; give the handler the stack frame
    mov rsi, %1           ; give the intrerrupt number
    call arch_isr_handler ; call the general handler
    POP_REG               ; restore registers
    add rsp, 8            ; clean up error code on the stack
    iretq                 ; return to previous context
%endmacro

; generate all 256 handlers
%assign i 0
%rep 256
GEN_HANDLER i
%assign i i+1
%endrep

; populate an array with those
section .data
arch_isr_handlers:
%assign i 0
%rep 256
    dq __isr_entry%+i
%assign i i+1
%endrep