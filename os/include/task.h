#ifndef MY_RISCV_OS_TASK_H
#define MY_RISCV_OS_TASK_H

#include "os.h"
#include "address.h"

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
    TaskContext task_context;
    uint64_t trap_cx_ppn; // trap上下文所在的物理地址
    uint64_t base_size; // 应用数据大小
    uint64_t kstack; // 应用的内核栈的虚拟地址
    uint64_t ustack; // 应用的用户栈的虚拟地址
    uint64_t entry; // 应用程序入口的地址
    PageTable page_table; // 应用页表所在的物理页
} TaskControlBlock;

#endif //MY_RISCV_OS_TASK_H