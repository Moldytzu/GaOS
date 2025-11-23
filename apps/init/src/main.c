#include <stdint.h>
#include <stddef.h>

#define SYS_WRITE 0
#define SYS_OPEN 1
#define SYS_CLOSE 2
#define SYS_READ 3
#define SYS_FORK 4
#define SYS_YIELD 5
#define SYS_WAITPID 6
#define SYS_EXIT 7
#define SYS_LSEEK 8

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTBLK 15
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34
#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define EWOULDBLOCK EAGAIN
#define ENOMSG 42
#define EIDRM 43
#define ECHRNG 44
#define EL2NSYNC 45
#define EL3HLT 46
#define EL3RST 47
#define ELNRNG 48
#define EUNATCH 49
#define ENOCSI 50
#define EL2HLT 51
#define EBADE 52
#define EBADR 53
#define EXFULL 54
#define ENOANO 55
#define EBADRQC 56
#define EBADSLT 57
#define EDEADLOCK EDEADLK
#define EBFONT 59
#define ENOSTR 60
#define ENODATA 61
#define ETIME 62
#define ENOSR 63
#define ENONET 64
#define ENOPKG 65
#define EREMOTE 66
#define ENOLINK 67
#define EADV 68
#define ESRMNT 69
#define ECOMM 70
#define EPROTO 71
#define EMULTIHOP 72
#define EDOTDOT 73
#define EBADMSG 74
#define EOVERFLOW 75
#define ENOTUNIQ 76
#define EBADFD 77
#define EREMCHG 78
#define ELIBACC 79
#define ELIBBAD 80
#define ELIBSCN 81
#define ELIBMAX 82
#define ELIBEXEC 83
#define EILSEQ 84
#define ERESTART 85
#define ESTRPIPE 86
#define EUSERS 87
#define ENOTSOCK 88
#define EDESTADDRREQ 89
#define EMSGSIZE 90
#define EPROTOTYPE 91
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP 95
#define ENOTSUP EOPNOTSUPP
#define EPFNOSUPPORT 96
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define EADDRNOTAVAIL 99
#define ENETDOWN 100
#define ENETUNREACH 101
#define ENETRESET 102
#define ECONNABORTED 103
#define ECONNRESET 104
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ESHUTDOWN 108
#define ETOOMANYREFS 109
#define ETIMEDOUT 110
#define ECONNREFUSED 111
#define EHOSTDOWN 112
#define EHOSTUNREACH 113
#define EALREADY 114
#define EINPROGRESS 115
#define ESTALE 116
#define EUCLEAN 117
#define ENOTNAM 118
#define ENAVAIL 119
#define EISNAM 120
#define EREMOTEIO 121
#define EDQUOT 122
#define ENOMEDIUM 123
#define EMEDIUMTYPE 124
#define ECANCELED 125
#define ENOKEY 126
#define EKEYEXPIRED 127
#define EKEYREVOKED 128
#define EKEYREJECTED 129
#define EOWNERDEAD 130
#define ENOTRECOVERABLE 131
#define ERFKILL 132
#define EHWPOISON 133
#define EMAXLEVEL 255

#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_APPEND (1 << 2)
#define O_CREAT (1 << 3)
#define O_DSYNC (1 << 4)
#define O_EXCL (1 << 5)
#define O_NOCTTY (1 << 6)
#define O_NONBLOCK (1 << 7)
#define O_RSYNC (1 << 8)
#define O_SYNC (1 << 9)
#define O_TRUNC (1 << 10)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int64_t _syscall(uint64_t number, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

int64_t sys_write(uint64_t fd, char *buffer, size_t size)
{
    return _syscall(SYS_WRITE, fd, (uint64_t)buffer, (uint64_t)size, 0, 0);
}

int64_t sys_read(uint64_t fd, char *buffer, size_t size)
{
    return _syscall(SYS_READ, fd, (uint64_t)buffer, (uint64_t)size, 0, 0);
}

int64_t sys_open(char *filename, uint64_t mode)
{
    return _syscall(SYS_OPEN, (uint64_t)filename, mode, 0, 0, 0);
}

int64_t sys_close(uint64_t fd)
{
    return _syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}

int64_t sys_fork()
{
    return _syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

void sys_yield()
{
    _syscall(SYS_YIELD, 0, 0, 0, 0, 0);
}

int64_t sys_waitpid(int64_t pid, int *stat_loc, int options)
{
    return _syscall(SYS_WAITPID, pid, (uint64_t)stat_loc, options, 0, 0);
}

int64_t sys_lseek(uint64_t fd, int64_t offset, int whence)
{
    return _syscall(SYS_LSEEK, fd, offset, whence, 0, 0);
}

void sys_exit(int code)
{
    _syscall(SYS_EXIT, (uint64_t)code, 0, 0, 0, 0);
}

size_t strlen(const char *str)
{
    size_t i = 0;
    while (*str)
    {
        str++;
        i++;
    }
    return i;
}

void puts(const char *str)
{
    sys_write(STDOUT, (char *)str, strlen(str));
    sys_yield();
}

int64_t open_read_fd(char *filename)
{
    int64_t fd = sys_open(filename, 0);
    char fd_str[] = {'0' + fd, 0};
    sys_write(STDOUT, fd_str, 1);
    return fd;
}

void test_open_close()
{
    sys_write(STDOUT, "\n", 1);

    // should print "34445"
    open_read_fd("/test.txt");
    sys_close(open_read_fd("/init.elf"));
    sys_close(open_read_fd("/test.txt"));
    open_read_fd("/init.elf");
    open_read_fd("/init.elf");

    sys_write(STDOUT, "\n", 1);
}

void test_read()
{
    int fd = sys_open("/test.txt", 0);
    char text[128];

    sys_lseek(fd, 5, SEEK_SET); // skip over this

    // read 8 chunks of 8 bytes each and display them
    for (int i = 1; i <= 8; i++)
    {
        sys_read(fd, text, 8);
        sys_write(STDOUT, text, 8);
    }

    sys_close(fd);
}

void test_fb()
{
    // create a striped pattern
    uint32_t to_write[4 * 1024];

    uint64_t fb_fd = sys_open("/dev/fb0", 0);

    for (int i = 0; i < 4 * 1024; i++)
        to_write[i] = i << (i % 43);

    for (int i = 0; i <= 4; i++)
        sys_write(fb_fd, (char *)to_write, 4 * 1024 * 4);
}

void test_fork()
{
    if (sys_fork() == 0) /// spawn pid 3
    {
        if (sys_fork() == 0) // spawn pid 4
        {
            sys_exit(7); // exit with code 7
        }
        while (1)
            ;
    }
    else
    {
        int status, pid;
        while ((pid = sys_waitpid(4, &status, 0)) < 0)
            sys_yield();

        puts("Child process ");
        char pid_str[] = {'0' + pid, 0};
        puts(pid_str);
        puts(" terminated with exit code ");
        pid_str[0] = '0' + (status & 0xFF);
        puts(pid_str);
        puts("\n");
    }
}

int _start()
{
    // we run as PID 2
    // PID 1 is the kernel io task

    // open stdin, stdout, stderr
    sys_open("/dev/console", 0);
    sys_open("/dev/console", 0);
    sys_open("/dev/console", 0);

    puts("Init process started\n");

    test_fork();
    test_fb();
    test_open_close();
    test_read();

    puts("Init finished\n");

    while (1)
        ;
    return 0;
}