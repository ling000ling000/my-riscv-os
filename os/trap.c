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
    reg_t scause = r_scause();

    // 根据 scause 的值进行分支处理
    switch (scause)
    {
        // 如果 scause 为 8，在 RISC-V 中通常表示“来自 U-mode 的 Environment Call”（即系统调用）
    case 8:
        {
            cx->sepc += 4;
            __SYSCALL(cx->a7, cx->a0, cx->a1, cx->a2);
            break;
        }
    default:
        {
            printf("scause=%lx sepc=%lx stval=%lx sstatus=%lx\n", r_scause(), r_sepc(), r_stval(), r_sstatus());
            panic("unkonwn scause: %d\n", scause);
            break;
        }
    }

    // 更新 sepc (Supervisor Exception Program Counter) 寄存器
    // 此处将程序计数器加 8，目的是跳过触发 Trap 的那条指令（如 ecall），防止死循环
    // 注意：标准 ecall 指令通常长 4 字节，此处加 8 属于该特定 OS 实现的逻辑
    // cx->sepc += 8;

    // 执行上下文恢复操作（将寄存器值恢复到 CPU）
    // __restore(cx);

    return cx;
}

void trap_init()
{
    w_stvec((reg_t)__alltraps);
}