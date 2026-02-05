//
// Created by kk on 2026/2/4.
//
#include <stddef.h>
#include "os.h"
#define USER_STACK_SIZE (4096 * 2)
#define KERNEL_STACK_SIZE (4096 * 2)

size_t syscall(size_t id, reg_t arg1, reg_t arg2, reg_t arg3)
{
    long ret;
    asm volatile(
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
    printf("syscall done");
    return ret;
}

void testsys()
{
    syscall(2, 3, 4, 5);
    syscall(1, 1, 1, 1);
    syscall(1, 2, 3, 4);
    while (1) {}
}

uint8_t kernel_stack[USER_STACK_SIZE];
uint8_t user_stack[KERNEL_STACK_SIZE];