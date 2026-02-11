//
// Created by kk on 2026/2/5.
//

#ifndef MY_RISCV_OS_CONTEXT_H
#define MY_RISCV_OS_CONTEXT_H

#include "../os.h"

// S Mode的 trap 上下文
typedef struct pt_reg {
    reg_t x0;  // x0: 恒为 0 寄存器。虽然硬件上恒为0，但为了栈布局对齐或统一宏处理，通常也会预留位置
    reg_t ra;  // x1 (Return Address): 返回地址
    reg_t sp;  // x2 (Stack Pointer): 栈指针
    reg_t gp;  // x3 (Global Pointer): 全局指针
    reg_t tp;  // x4 (Thread Pointer): 线程指针 (通常用于指向当前 CPU 的结构体或线程控制块)
    reg_t t0;  // x5: 临时寄存器
    reg_t t1;  // x6: 临时寄存器
    reg_t t2;  // x7: 临时寄存器
    reg_t s0;  // x8 (Saved Register/Frame Pointer): 保存寄存器/帧指针
    reg_t s1;  // x9: 保存寄存器
    reg_t a0;  // x10: 函数参数 / 返回值
    reg_t a1;  // x11: 函数参数 / 返回值
    reg_t a2;  // x12: 函数参数
    reg_t a3;  // x13: 函数参数
    reg_t a4;  // x14: 函数参数
    reg_t a5;  // x15: 函数参数
    reg_t a6;  // x16: 函数参数
    reg_t a7;  // x17: 函数参数 / 系统调用号
    reg_t s2;  // x18: 保存寄存器 (调用者保存)
    reg_t s3;  // x19: 保存寄存器
    reg_t s4;  // x20: 保存寄存器
    reg_t s5;  // x21: 保存寄存器
    reg_t s6;  // x22: 保存寄存器
    reg_t s7;  // x23: 保存寄存器
    reg_t s8;  // x24: 保存寄存器
    reg_t s9;  // x25: 保存寄存器
    reg_t s10; // x26: 保存寄存器
    reg_t s11; // x27: 保存寄存器
    reg_t t3;  // x28: 临时寄存器
    reg_t t4;  // x29: 临时寄存器
    reg_t t5;  // x30: 临时寄存器
    reg_t t6;  // x31: 临时寄存器

    // S模式下的特殊控制状态寄存器 (CSR)
    reg_t sstatus;
    reg_t sepc;
} pt_reg_t;



// s mode的任务上下文
typedef struct TaskContext
{
    reg_t ra;
    reg_t sp;
    reg_t s0;
    reg_t s1;
    reg_t s2;
    reg_t s3;
    reg_t s4;
    reg_t s5;
    reg_t s6;
    reg_t s7;
    reg_t s8;
    reg_t s9;
    reg_t s10;
    reg_t s11;
} TaskContext;

#endif //MY_RISCV_OS_CONTEXT_H