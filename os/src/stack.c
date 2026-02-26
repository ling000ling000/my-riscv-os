#include "../include/stack.h"
#include "../include/stdio.h"

// 初始化
void initStack(Stack *stack)
{
    stack->top = -1;
}

bool isEmpty(Stack *stack)
{
    return stack->top == -1;
}

bool isFull(Stack *stack)
{
    return stack->top == MAX_SIZE - 1;
}

// 入栈操作
void push(Stack *stack, uint64_t data)
{
    if (isFull(stack)) // 先判断是否栈满
    {
        printk("stack overflow\n");
        return;
    }
    stack->top ++; // top自增
    stack->data[stack->top] = data;
}

// 出栈操作
uint64_t pop(Stack *stack)
{
    if (isEmpty(stack))
    {
        printk("stack underflow\n");
        return -1;
    }
    return stack->data[stack->top --]; // 先返回top指向的元素，再top自减
}

// 获取栈顶元素
uint64_t top(Stack *stack)
{
    if (isEmpty(stack))
    {
        printk("stack is empty\n");
        return -1;
    }
    return stack->data[stack->top];
}
