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

| Syscall name | rdi | rsi | rdx | r10 | r8 | r9 | rax | description |
|--------------|-----|-----|-----|-----|----|----|-----|-------------|
| write |  0  | uint64_t fd | char *buffer | size_t size | size_t file_offset | - | error code | Write (size) bytes from buffer to (fd). |
| open |  1  | char *filename | uint64_t mode | - | - | - | error code or fd | Open file (filename) with (mode). |
| close | 2 | uint64_t fd | - | - | - | - | error code | Closes file (fd). |
| read | 3 | uint64_t fd | char *buffer | size_t size | size_t file_offset | - | error code | Read (size) bytes from (fd) to (buffer). |