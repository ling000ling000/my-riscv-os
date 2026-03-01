#ifndef MY_RISCV_OS_TASK_H
#define MY_RISCV_OS_TASK_H

#include "os.h"
#include "address.h"

// 定义用户栈大小
#define USER_STACK_SIZE (4096 * 2)
// 定义内核栈大小
#define KERNEL_STACK_SIZE (4096 * 2)
// 定义最大任务数
#define MAX_TASKS 10

typedef enum TaskState
{
    UnInit,
    Ready,
    Running,
    Exited,
} TaskState;

typedef struct TaskControlBlock
{
    TaskState task_state;
    int pid;
    struct TaskControlBlock* parent; // 父进程的指针
    TaskContext task_context;
    uint64_t trap_cx_ppn; // trap上下文所在的物理地址
    uint64_t base_size; // 应用数据大小
    uint64_t kstack; // 应用的内核栈的虚拟地址
    uint64_t ustack; // 应用的用户栈的虚拟地址
    uint64_t entry; // 应用程序入口的地址
    PageTable page_table; // 应用页表所在的物理页
} TaskControlBlock;

int alloc_pid();

#endif //MY_RISCV_OS_TASK_H