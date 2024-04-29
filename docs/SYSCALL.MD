# System calls

## x86_64
### Calling convention

| Register | Description |
|----------|-------------|
| rax | Syscall number |
| rax | Return value |
| rdi | First parameter |
| rsi | Second parameter |
| rdx | Third parameter |
| r8 | Fourth parameter |
| r9 | Fifth parameter |

### Syscall Table

| Syscall name | rax | rdi | rsi | rdx | r8 | r9 |
|--------------|-----|-----|-----|-----|----|----|
| write        |  0  | uint64_t fd | char *buffer | size_t size | - | - |