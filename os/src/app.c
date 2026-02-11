//
// Created by kk on 2026/2/7.
//
#include "../include/os.h"

size_t syscall(size_t id, reg_t arg1, reg_t arg2, reg_t arg3)
{
    long ret;
    asm volatile(
        /*
        * a7 放 syscall number
        * a0=a fd, a1=buf, a2=len
        * 返回值放 a0
         */
        "mv a7, %1\n\t"
        "mv a0, %2\n\t"
        "mv a1, %3\n\t"
        "mv a2, %4\n\t"
        "ecall\n\t"
        "mv %0, a0"
        : "=r" (ret)
        : "r" (id), "r" (arg1), "r" (arg2), "r" (arg3)
        : "a7", "a0", "a1", "a2", "memory"
    );
    return ret;
}

size_t sys_write(size_t fd, const char* buf, size_t len)
{
    syscall(__NR_write, fd, buf, len);
}

size_t sys_yield()
{
    syscall(__NR_shced_yield, 0, 0, 0);
}

void task_delay(volatile int count)
{
    count *= 50000;
    while (count--);
}

uint64_t sys_get_time()
{
    return syscall(__NR_gettimeofday, 0, 0, 0);
}

void task1()
{
    const char *message = "task1 is running!\n";
    int len = strlen(message);
    while (1)
    {
        sys_write(1, message, len);
        task_delay(10000);
        sys_yield();
    }
}


void task2()
{
    const char *message = "task2 is running!\n";
    int len = strlen(message);
    while (1)
    {
        sys_write(1, message, len);
        // task_delay(10000);
        // sys_yield();
    }
}

void task3()
{
    const char *message = "task3 is running!\n";
    int len = strlen(message);
    while (1) {
        uint64_t start = sys_get_time();
        uint64_t end = start + 50000;   // 50ms 或 50,000us 取决于你的时间单位

        while (sys_get_time() < end) {
            sys_write(1, message, len);
            sys_yield();
        }
    }
}


void task_init(void)
{
    task_create(task1);
    task_create(task2);
    task_create(task3);
}