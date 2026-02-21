#ifndef MY_RISCV_OS_STACK_H
#define MY_RISCV_OS_STACK_H

#include "types.h"

#define MAX_SIZE 10000

typedef struct
{
    uint64_t data[MAX_SIZE];
    int top; // 栈顶指针
} Stack;

// 栈操作
void initStack(Stack *stack);

bool isEmpty(Stack* stack);

bool isFull(Stack* stack);

void push(Stack* stack, uint64_t data);

uint64_t pop(Stack* stack);

uint64_t top(Stack* stack);

#endif //MY_RISCV_OS_STACK_H
