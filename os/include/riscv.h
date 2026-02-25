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

/**
 * @brief 写入SATP（Supervisor Address Translation and Protection）寄存器
 * @param x 要写入SATP寄存器的64位数值（包含分页模式和页表根PPN）
 * @note 该函数是RISC-V特权指令csrw的封装，仅S态（监管态）可执行
 *       asm volatile 确保汇编指令不被编译器优化，保证执行顺序和完整性
 */
static inline void w_satp(reg_t x)
{
    // 内联汇编格式：asm volatile("汇编指令模板" : 输出操作数 : 输入操作数 : 破坏描述符)
    // csrw satp, %0：将通用寄存器中的值（%0对应输入参数x）写入satp特权寄存器
    // "r" (x)：表示将x放入任意通用寄存器（r约束），作为汇编指令的输入
    asm volatile("csrw satp, %0" : : "r" (x));
}

/**
 * @brief 读取SATP寄存器的值
 * @return 从SATP寄存器读取的64位数值
 * @note 该函数是RISC-V特权指令csrr的封装，仅S态（监管态）可执行
 */
static inline reg_t r_satp()
{
    // 定义临时变量x，用于存储从SATP寄存器读取的值
    reg_t x;
    // csrr %0, satp：将satp特权寄存器的值读取到通用寄存器（%0对应变量x）
    // "=r" (x)：表示将汇编指令的结果写入变量x（=表示输出，r约束通用寄存器）
    asm volatile("csrr %0, satp" : "=r" (x) );
    return x;
}

/**
 * @brief 刷新TLB（Translation Lookaside Buffer，地址转换旁路缓存）
 * @note 该函数封装RISC-V的sfence.vma指令，用于清空TLB中的无效映射条目
 *       确保页表修改后，CPU使用最新的地址映射关系
 */
static inline void sfence_vma()
{
    // the zero, zero means flush all TLB entries.
    // （原注释：两个zero参数表示刷新所有TLB条目，不限制地址/ASID）

    // sfence.vma：RISC-V的TLB刷新指令，格式为sfence.vma [rs1], [rs2]
    // rs1=zero（x0寄存器，值恒为0）：表示刷新所有虚拟地址的TLB条目
    // rs2=zero（x0寄存器）：表示刷新所有地址空间标识符（ASID）的TLB条目
    // 组合效果：清空当前核的所有TLB缓存，适用于内核页表全局刷新场景
    asm volatile("sfence.vma zero, zero");
}


#endif //MY_RISCV_OS_RISCV_H