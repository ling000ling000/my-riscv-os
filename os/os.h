#ifndef __OS_H__
#define __OS_H__

#include <stddef.h>
#include <stdarg.h>

#include "types.h"
#include "context.h"
#include "riscv.h"

// printf.c
extern int printf(const char *format, ...);
extern void panic(const char *fmt, ...);
extern void sbi_console_putchar(int ch);

// batch.c
extern void testsys();
extern void app_init_context();

// trap.c
extern void trap_init();

// syscall
void __SYSCALL(size_t syscall_id, reg_t arg1, reg_t arg2, reg_t arg3);
#define sys_write 64

#endif // __OS_H__