# System calls

## x86_64
### Calling convention

| Register | Description |
|----------|-------------|
| rdi | Syscall number |
| rax | Return value |
| rsi | First parameter |
| rdx | Second parameter |
| r10 | Third parameter |
| r8 | Fourth parameter |
| r9 | Fifth parameter |

### Syscall Table

| Syscall name | rdi | rsi | rdx | r10 | r8 | r9 |
|--------------|-----|-----|-----|-----|----|----|
| write        |  0  | uint64_t fd | char *buffer | size_t size | - | - |