bits 64

section .text

global arch_syscall_entry
extern syscall_handlers, syscall_count

arch_syscall_entry:
    ; check if the requested syscall is in bounds
    cmp rdi, [syscall_count]
    jae .out_of_bounds

    ; first, swap the stack to a known one
    mov rax, gs:0x0     ; read context type
    cmp eax, 0x55555555 ; compare with cpu magic type
    jne .swap_stack     ; if it doesn't equal to it go switch stacks directly
    swapgs              ; else, swap the context first

.swap_stack:
    mov gs:0x10, rsp ; save the userspace stack
    mov rsp, gs:0x8  ; load our new stack

    ; r11 holds old rflags
    ; rcx holds old rip
    push r11
    push rcx

    ; prepare the parameter registers
    ; parameter   : num  1   2   3   4  5
    ; sysv abi    : rdi rsi rdx rcx r8 r9
    ; syscall abi : rdi rsi rdx r10 r8 r9
    mov rcx, r10

    ; here rdi holds the syscall number
    lea r11, syscall_handlers ; get the address of the handlers
    call [r11 + rdi * 8]      ; call the pointed function (base + offset * sizeof(uint64_t))

    ; check again the context type
    mov rcx, gs:0x0          ; read context type
    cmp ecx, 0x55555555      ; compare with the cpu magic type
    jne .switch_to_userspace ; if it isn't then switch directly to userspace
    swapgs                   ; else, switch to task context

.switch_to_userspace:
    ; pop old rflags and rip
    pop rcx
    pop r11

    mov rsp, gs:0x10 ; restore the userspace stack

.out_of_bounds:
    o64 sysret