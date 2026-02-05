//
// Created by kk on 2026/2/4.
//

#ifndef MY_RISCV_OS_RISCV_H
#define MY_RISCV_OS_RISCV_H

#include "os.h"

/* 读取 sepc 寄存器的值
 * sepc (Supervisor Exception Program Counter):
 * 记录发生异常或中断时的指令地址（或下一条指令地址），用于处理完异常后返回原程序。
 */
static inline reg_t r_spec()
{
    reg_t x;
    asm volatile ("csrr %0, sepc" : "=r"(x));
    return x;
}

/* 读取 scause 寄存器的值
 * scause (Supervisor Cause Register):
 * 记录导致异常或中断的具体原因（通过最高位的 Interrupt 标志和低位的 Exception Code 区分）。
 */
static inline reg_t r_scauese()
{
    reg_t x;
    asm volatile ("csrr %0, scause" : "=r"(x));
    return x;
}

/* 读取 stval 寄存器的值
 * stval (Supervisor Trap Value):
 * 记录异常发生时的附加信息。例如在缺页异常中，它记录了导致错误的虚拟内存地址。
 */
static inline reg_t r_sstatus()
{
    reg_t x;
    asm volatile ("csrr %0, sstatus" : "=r"(x));
    return x;
}

/* 写入 sstatus 寄存器的值
 * 用于修改处理器状态，例如开启/关闭中断。
 */
static inline void w_sstatus(reg_t x)
{
    // csrw (Control Status Register Write): 将通用寄存器的值写入 sstatus
    // "r" (x) 表示将变量 x 的值放入任意通用寄存器作为输入
    asm volatile ("csrw sstatus, %0" : : "r"(x));
}

/* 写入 stvec 寄存器的值
 * stvec (Supervisor Trap Vector):
 * 设置异常处理程序的入口地址。当发生异常时，PC 指针会跳转到此寄存器指向的地址。
 */
static inline void w_stvec(reg_t x)
{
    asm volatile ("csrw stvec, %0" : : "r"(x));
}

/* 读取 stvec 寄存器的值 */
static inline reg_t r_stvec()
{
    reg_t x;
    asm volatile ("csrr %0, stvec" : "=r"(x));
    return x;
}


#endif //MY_RISCV_OS_RISCV_H