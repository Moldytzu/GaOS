#define MODULE "syscall"
#include <misc/logger.h>

#include <arch/arch.h>
#include <arch/x86_64/msr.h>

#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_FMASK 0xC0000084
#define IA32_EFER 0xC0000080

extern void arch_syscall_entry(void);

uint64_t arch_syscall_handler(uint64_t param1, uint64_t param2, uint64_t param3, uint64_t number, uint64_t param4, uint64_t param5)
{
    log_info("number %d: %p %p %p %p %p", number, param1, param2, param3, param4, param5);

    return 1;
}

void arch_syscall_init(void)
{
    // enable sysret/syscall
    wrmsr(IA32_EFER, rdmsr(IA32_EFER) | 1);

    /*
    set target cs/ss pairs

    docs: Intel SDM Volume 3 5.8.8
    for syscall: (userspace -> kernelspace)
        code segemnt is IA32_STAR[47:32]
        stack segment is IA32_STAR[47:32] + 8

    for sysret: (kernelspace -> userspace)
        code segment is IA32_STAR[63:48] + 16
        stack segment is IA32_STAR[63:48] + 8

    */

    wrmsr(IA32_STAR, 0x0013000800000000);            // write cs/ss pairs for userspace and kernel
    wrmsr(IA32_LSTAR, (uint64_t)arch_syscall_entry); // write the syscall handler entry point
}