#ifndef MY_RISCV_OS_STDIO_H
#define MY_RISCV_OS_STDIO_H

#include "os.h"

int _vsnprintf(char * out, size_t n, const char* s, va_list vl);
void panic(const char *fmt, ...);
int printk(const char* s, ...);
int printf(const char* s, ...);

/* 文件描述符 */
typedef enum std_fd_t
{
    stdin,
    stdout,
    stderr,
} std_fd_t;

#endif //MY_RISCV_OS_STDIO_H