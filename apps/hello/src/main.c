#include <stdint.h>
#include <stddef.h>

#define SYS_WRITE 0
#define SYS_OPEN 1
#define SYS_CLOSE 2
#define SYS_READ 3

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int64_t _syscall(uint64_t number, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

int64_t sys_write(uint64_t fd, char *buffer, size_t size, size_t file_offset)
{
    return _syscall(SYS_WRITE, fd, (uint64_t)buffer, (uint64_t)size, file_offset, 0);
}

int64_t sys_read(uint64_t fd, char *buffer, size_t size, size_t file_offset)
{
    return _syscall(SYS_READ, fd, (uint64_t)buffer, (uint64_t)size, file_offset, 0);
}

int64_t sys_open(char *filename, uint64_t mode)
{
    return _syscall(SYS_OPEN, (uint64_t)filename, mode, 0, 0, 0);
}

int64_t sys_close(uint64_t fd)
{
    return _syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}

int64_t open_read_fd(char *filename)
{
    int64_t fd = sys_open(filename, 0);
    char fd_str[] = {'0' + fd, 0};
    sys_write(0, fd_str, 1, 0);
    return fd;
}

void test_open_close()
{
    sys_write(0, "\n", 1, 0);

    // should print "34445"
    open_read_fd("/initrd/test.txt");
    sys_close(open_read_fd("/initrd/hello.elf"));
    sys_close(open_read_fd("/initrd/test.txt"));
    open_read_fd("/initrd/hello.elf");
    open_read_fd("/initrd/hello.elf");

    sys_write(0, "\n", 1, 0);
}

void test_read()
{
    int fd = sys_open("/initrd/test.txt", 0);
    char text[128];
    sys_read(fd, text, 128, 0);
    sys_write(0, text, 128, 0);
    sys_close(fd);
}

int _start()
{
    sys_write(STDOUT, "Hello, Gallium!", 15, 0); // write a message

    test_open_close();
    test_read();

    while (1)
        ;
    return 0;
}