#ifndef __OS_H__
#define __OS_H__

#include <stddef.h>
#include <stdarg.h>

#include "types.h"
#include "riscv.h"

extern int printf(const char *format, ...);
extern void panic(const char *fmt, ...);
extern void sbi_console_putchar(int ch);

extern void testsys();

#endif // __OS_H__