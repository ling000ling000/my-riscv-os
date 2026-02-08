//
// Created by kk on 2026/2/5.
//

#ifndef MY_RISCV_OS_CONTEXT_H
#define MY_RISCV_OS_CONTEXT_H

#include "os.h"

// S Mode的 trap 上下文
typedef struct pt_reg {
    reg_t x0;       // 0(sp)   预留
    reg_t ra;       // 8(sp)
    reg_t sscratch; // 16(sp)  注意：这里不是 sp，是 sscratch 交换出来的值
    reg_t gp;       // 24(sp)
    reg_t tp;       // 32(sp)
    reg_t t0;       // 40(sp)
    reg_t t1;       // 48(sp)
    reg_t t2;       // 56(sp)
    reg_t s0;       // 64(sp)
    reg_t s1;       // 72(sp)
    reg_t a0;       // 80(sp)
    reg_t a1;       // 88(sp)
    reg_t a2;       // 96(sp)
    reg_t a3;       // 104(sp)
    reg_t a4;       // 112(sp)
    reg_t a5;       // 120(sp)
    reg_t a6;       // 128(sp)
    reg_t a7;       // 136(sp)
    reg_t s2;       // 144(sp)
    reg_t s3;       // 152(sp)
    reg_t s4;       // 160(sp)
    reg_t s5;       // 168(sp)
    reg_t s6;       // 176(sp)
    reg_t s7;       // 184(sp)
    reg_t s8;       // 192(sp)
    reg_t s9;       // 200(sp)
    reg_t s10;      // 208(sp)
    reg_t s11;      // 216(sp)
    reg_t t3;       // 224(sp)
    reg_t t4;       // 232(sp)
    reg_t t5;       // 240(sp)
    reg_t t6;       // 248(sp)
    reg_t sstatus;  // 256(sp)
    reg_t sepc;     // 264(sp)
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