#include <stdint.h>
#include <stddef.h>

#define SYS_WRITE 0
#define SYS_OPEN 1
#define SYS_CLOSE 2

int64_t _syscall(uint64_t number, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

int64_t sys_write(uint64_t fd, char *buffer, size_t size)
{
    return _syscall(SYS_WRITE, fd, (uint64_t)buffer, (uint64_t)size, 0, 0);
}

int64_t sys_open(char *filename, uint64_t mode)
{
    return _syscall(SYS_OPEN, (uint64_t)filename, mode, 0, 0, 0);
}

int64_t sys_close(uint64_t fd)
{
    return _syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}

int64_t open_write_fd(char *filename)
{
    int64_t fd = sys_open(filename, 0);
    char fd_str[] = {'0' + fd, 0};
    sys_write(0, fd_str, 1);
    return fd;
}

void test_fd()
{
    sys_write(0, "\n", 1);

    // should print "34445"
    open_write_fd("/initrd/test.txt");
    sys_close(open_write_fd("/initrd/hello.elf"));
    sys_close(open_write_fd("/initrd/test.txt"));
    open_write_fd("/initrd/hello.elf");
    open_write_fd("/initrd/hello.elf");

    sys_write(0, "\n", 1);
}

int _start()
{
    sys_write(0, "Hello, Gallium!", 15); // write a message

    test_fd();

    while (1)
        ;
    return 0;
}