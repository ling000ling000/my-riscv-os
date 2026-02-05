//
// Created by kk on 2026/2/5.
//
#include "os.h"
#include "context.h"
#include "riscv.h"

// 声明外部符号 __alltraps
// 这个符号在汇编文件 (entry.S) 中定义，是所有中断/异常的统一汇编入口
extern void __alltraps(void);
extern void __restore(pt_reg_t *next);

/* Trap 处理主函数
 * 参数 cx: 指向内核栈上保存的上下文结构体 (TrapContext) 的指针
 * 该指针由 __alltraps 中的 "mv a0, sp" 指令传递而来
 */
pt_reg_t* trap_handler(pt_reg_t* cx)
{
    // 读取 scause (Supervisor Cause) 寄存器
    // 该寄存器包含了一个数字，指示了当前陷入内核的具体原因 (是时钟中断、非法指令还是系统调用等)
    reg_t scause = r_scauese();

    // 打印关键调试信息
    // %x 以十六进制格式输出，方便查看位模式
    printf("cause:%x\n", scause);    // 输出异常原因
    printf("a0:%x\n", cx->a0);       // 输出寄存器 a0 (通常存放系统调用参数或返回值)
    printf("a1:%x\n", cx->a1);       // 输出寄存器 a1 (系统调用参数)
    printf("a2:%x\n", cx->a2);       // 输出寄存器 a2 (系统调用参数)
    printf("a7:%x\n", cx->a7);       // 输出寄存器 a7 (RISC-V 约定中通常存放 System Call ID)
    printf("sepc:%x\n", cx->sepc);   // 输出异常发生时的指令地址 (用于回溯代码位置)
    printf("sstatus:%x\n", cx->sstatus); // 输出发生异常时的状态 (S/U 模式，中断使能位等)
    printf("sp:%x\n", cx->sp);       // 输出发生异常时的栈指针

    if ((scause & 0xfff) == 8) {        // U-mode ecall
        cx->sepc += 4;                  // 只跳过 ecall 一条指令
        printf("ecall handled, return to 0x%lx\n", (unsigned long)cx->sepc);

        // TODO: 在这里根据 a7 分发 syscall，并把返回值写回 cx->a0
        return cx;
    }

    // 其他异常先直接停机，别硬跳 sepc
    printf("unhandled trap, halt.\n");
    while (1) {}

    return cx;
}

void trap_init()
{
    w_stvec((reg_t)__alltraps);
}