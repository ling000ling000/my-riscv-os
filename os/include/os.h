#ifndef __OS_H__
#define __OS_H__

#include <stddef.h>
#include <stdarg.h>

#include "types.h"
#include "context.h"
#include "riscv.h"
#include "task.h"
#include "stdio.h"
#include "address.h"

// printf.c
// extern int printk(const char *format, ...);
// extern void panic(const char *fmt, ...);
// extern void sbi_console_putchar(int ch);

// sbi.c
extern void sbi_console_putchar(int ch);
int sbi_console_getchar(void);

// kerneltrap.S
extern void __alltraps(void);
extern void __restore(pt_reg_t *next);

// batch.c
extern void testsys();
extern void app_init_context();

// trap.c
extern void trap_init();
extern void trap_return();
extern void trap_handler();
extern void set_kernel_trap_entry();

// syscall
uint64_t __SYSCALL(size_t syscall_id, reg_t arg1, reg_t arg2, reg_t arg3);
#define __NR_write 64
#define __NR_shced_yield 124
#define __NR_gettimeofday 169
#define __NR_read 63
#define __NR_clone 220
#define __NR_execve 221
#define __NR_exit 93
#define __NR_waitid 95
uint64_t sys_write(size_t fd, const char* buf, size_t len);
uint64_t sys_yield();
uint64_t sys_get_time();
int sys_fork();
int sys_exit(uint64_t exit_code);
char* translated_byte_buffer(const char* data , size_t len);
int sys_exec(const char* name);
int sys_waitpid(int *status);

// switch.S
extern void __switch(TaskContext *current_task_cx_ptr, TaskContext* next_task_cx_ptr);

// task.c
#define MAX_TASKS 10
extern void schedule();
extern void task_create(void (*task_entry)(void));
extern void run_first_task();
TaskControlBlock*  task_create_pt(size_t app_id); /* 创建应用页表 */
void proc_mapstacks(PageTable* kpgtbl); /* 映射用户程序内核栈 */
uint64_t get_current_trap_cx();
uint64_t current_user_token();
void app_init(size_t app_id);
int __sys_fork();
void proc_init();
void exit_current_and_run_next(uint64_t exit_code);
void exec(const char* name);
int wait(uint64_t status_ptr);

// app.c
extern void task_init();
char getchar();

// timer.c
extern void sbi_set_timer(uint64_t stime_value);
extern void set_next_trigger();
extern uint64_t get_time_us();
extern void timer_init();

// string
extern size_t strlen(const char *s);
void* memset(void *dest, int ch, size_t count);
void* memcpy(void* dest, const void* src, size_t n);
int strcmp(const char *lhs, const char *rhs);
void strcat(char *dest, const char *src, int n);

#endif // __OS_H__
