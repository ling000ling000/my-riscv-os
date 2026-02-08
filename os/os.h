#ifndef __OS_H__
#define __OS_H__

#include <stddef.h>
#include <stdarg.h>

#include "types.h"
#include "context.h"
#include "riscv.h"
#include "task.h"

// printf.c
extern int printf(const char *format, ...);
extern void panic(const char *fmt, ...);
extern void sbi_console_putchar(int ch);

// kerneltrap.S
extern void __alltraps(void);
extern void __restore(pt_reg_t *next);

// batch.c
extern void testsys();
extern void app_init_context();

// trap.c
extern void trap_init();

// syscall
void __SYSCALL(size_t syscall_id, reg_t arg1, reg_t arg2, reg_t arg3);
#define __NR_write 64
#define __NR_shced_yield 124

// switch.S
extern void __switch(TaskContext *current_task_cx_ptr, TaskContext* next_task_cx_ptr);

// task.c
extern void schedule();
extern void task_create(void (*task_entry)(void));
extern void run_first_task();

// app.c
extern void task_init();


// string
extern size_t strlen(const char *s);

#endif // __OS_H__