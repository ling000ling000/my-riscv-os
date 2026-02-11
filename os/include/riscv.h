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
static inline reg_t r_sepc()
{
    reg_t x;
    asm volatile ("csrr %0, sepc" : "=r"(x));
    return x;
}

/* 读取 scause 寄存器的值
 * scause (Supervisor Cause Register):
 * 记录导致异常或中断的具体原因（通过最高位的 Interrupt 标志和低位的 Exception Code 区分）。
 */
static inline reg_t r_scause()
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

// stval 记录了trap发生时的地址
static inline reg_t r_stval()
{
    reg_t x;
    asm volatile("csrr %0, stval" : "=r" (x) );
    return x;
}

/* 读取 stvec 寄存器的值 */
static inline reg_t r_stvec()
{
    reg_t x;
    asm volatile ("csrr %0, stvec" : "=r"(x));
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


/* * 读取当前的时间值 (mtime)
 * 通常用于计算时间片或统计运行时间
 */
static inline reg_t r_mtime()
{
    reg_t x;
    asm volatile ("rdtime %0" : "=r"(x)); // rdtime: RISC-V 伪指令，用于读取 time CSR
    return x;
}

// 监管者模式中断使能位定义 (Supervisor Interrupt Enable Bits)

// 定义 SEIE (Supervisor External Interrupt Enable) 掩码
// 对应 sie 寄存器的第 9 位，控制 S 模式下的外部中断（如外设中断）
#define SIE_SEIE (1L << 9) // external

// 定义 STIE (Supervisor Timer Interrupt Enable) 掩码
// 对应 sie 寄存器的第 5 位，控制 S 模式下的时钟中断
#define SIE_STIE (1L << 5) // timer

// 定义 SSIE (Supervisor Software Interrupt Enable) 掩码
// 对应 sie 寄存器的第 1 位，控制 S 模式下的软件中断（常用于核间通信 IPI）
#define SIE_SSIE (1L << 1) // software

/* * 读取 sie 寄存器的当前值
 * static inline: 建议编译器内联展开，减少函数调用开销
 */
static inline reg_t r_sie()
{
    reg_t x;
    // 内联汇编指令
    // csrr (Control Status Register Read): 读取控制状态寄存器
    // %0: 对应输出操作数 x
    // sie: 要读取的目标寄存器名称
    // "=r" (x): 输出约束，"=" 表示只写，"r" 表示分配一个通用寄存器来存放结果 x
    asm volatile("csrr %0, sie" : "=r" (x) );
    return x;
}

/* * 向 sie 寄存器写入新值
 * x: 要写入的位掩码配置
 */
static inline void w_sie(reg_t x)
{
    // 内联汇编指令
    // csrw (Control Status Register Write): 写入控制状态寄存器
    // sie: 目标寄存器
    // %0: 对应输入操作数 x
    // "r" (x): 输入约束，表示将变量 x 的值放入一个通用寄存器中供指令使用
    asm volatile("csrw sie, %0" : : "r" (x));
}


#endif //MY_RISCV_OS_RISCV_H