bits 64

section .text

global arch_syscall_entry
extern syscall_handlers, syscall_count

arch_syscall_entry:
    ; check if the requested syscall is in bounds
    cmp rdi, [syscall_count]
    jae .out_of_bounds       ; it's out of bounds

    ; first, swap the stack to a known one
    mov rax, gs:0x0 ; read context type
    cmp rax, 0      ; if it equals to cpu
    je .swap_stack  ; swap the stack directly
    swapgs          ; else, swap the context first

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
    mov rax, rdi
    lea r11, syscall_handlers ; get the address of the handlers
    shl rax, 3                ; calculate the offset in bytes by multiplying by 8 (sizeof(uint64_t))
    call [r11 + rax]          ; call the pointed function (base + offset)

    pop rcx
    pop r11

    mov rsp, gs:0x10 ; restore the userspace stack

.out_of_bounds:
    o64 sysret