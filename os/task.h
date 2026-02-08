//
// Created by kk on 2026/2/8.
//

#ifndef MY_RISCV_OS_TASK_H
#define MY_RISCV_OS_TASK_H

#include "os.h"

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
} TaskControlBlock;

#endif //MY_RISCV_OS_TASK_H