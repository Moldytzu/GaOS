bits 64

section .text

%include "registers.mac"

global arch_syscall_entry
extern syscall_handlers, syscall_count, sys_fork

arch_sys_fork_entry:
    ; fill the stack with zeroes
    ; the arch_cpu_state_t structure includes the data iretq would require to return from an interrupt
    ; thus we have to simulate that here to prevent the stack being leaked
    xor rax, rax
    push rax
    push rax
    push rax
    push rax
    push rax
    push rax

    ; push all registers on stack
    PUSH_REG

    mov rdi, rsp
    call sys_fork

    add rsp, 8 * 22 ; clean up the stack
    ret

arch_syscall_entry:
    ; here the interrupts are disabled
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

    ; handle sys_fork elsewhere
    cmp rdi, 4
    jne .handle_normal_handler

    call arch_sys_fork_entry
    jmp .finish_handler

.handle_normal_handler:
    ; prepare the parameter registers
    ; parameter   : num  1   2   3   4  5
    ; sysv abi    : rdi rsi rdx rcx r8 r9
    ; syscall abi : rdi rsi rdx r10 r8 r9
    mov rcx, r10

    ; here rdi holds the syscall number
    lea r11, syscall_handlers ; get the address of the handlers
    
    sti                  ; enable interrupts      
    call [r11 + rdi * 8] ; call the pointed function (base + offset * sizeof(uint64_t))
    cli                  ; disable interrupts

.finish_handler:
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