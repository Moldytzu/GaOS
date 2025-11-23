#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <config.h>
#include <lock/spinlock.h>

// linux-compatible POSIX errors
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
#define EMAXLEVEL 255 // maximum error level
#define error_of(x) ((int64_t)(x))
#define is_error(x) (error_of(x) < 0 && error_of(x) >= -EMAXLEVEL)
#define error_ptr(x) ((void *)(x))

#define STACK_TRACE_WALK(depth) (__builtin_extract_return_addr(__builtin_return_address(depth)))

#define iasm asm volatile
#define ifunc [[gnu::always_inline]] static inline
#define bitsof(type) (sizeof(type) * 8)
#define pstruct typedef struct [[gnu::packed]]
#define align_addr(al) [[gnu::aligned(al)]]
#define used(x) (void)x

typedef long ssize_t;

void *memcpy(void *dest, const void *src, size_t n);
char *strcpy(char *dest, char *src);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(char *str);
void strrev(char *str);

void vprintk(const char *fmt, va_list list);
void vprintk_unsafe(const char *fmt, va_list list);
void printk(const char *fmt, ...);
void printk_unsafe(const char *fmt, ...);

void vprintk_serial(const char *fmt, va_list list);
void vprintk_serial_unsafe(const char *fmt, va_list list);
void printk_serial(const char *fmt, ...);
void printk_serial_unsafe(const char *fmt, ...);

noreturn void halt();

ifunc void zero64(uint64_t *s, size_t n)
{
    n /= sizeof(uint64_t);
    for (size_t i = 0; i < n; i++)
        s[i] = 0;
}

ifunc int min(int a, int b)
{
    if (a > b)
        return b;
    return a;
}

ifunc int max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

ifunc int abs(int a)
{
    if (a < 0)
        return -a;
    return a;
}