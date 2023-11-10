bits 64

section .text

global arch_spinlock_acquire, arch_spinlock_release

arch_spinlock_acquire: ; rdi = spinlock address 
	.acquire:
		lock bts qword [rdi], 0x0 ; try to aquire it
		jnc .exit                ; if carry bit is set then it's locked
	.spin:
        pause               ; hint spinlock
		bt qword [rdi], 0x0  ; test the bit again
		jc .spin            ; if it's still set try again
		jmp .acquire        ; else aquire it and exit
	.exit:
		ret

arch_spinlock_release: ; rdi = spinlock address
    lock btr qword [rdi], 0x0
    ret