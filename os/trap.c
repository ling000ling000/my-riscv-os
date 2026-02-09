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

    // 提取低12位作为异常中断编号，0xfff过滤高位的标志位
    reg_t cause_code = scause & 0xfff;

    // 判断是否为“中断” (Interrupt)
    // RISC-V 规定：如果 scause 的最高位（MSB）为 1，则表示这是一个异步中断
    // 0x8000...0000 是一个 64 位的数，最高位为 1，其余为 0
    if (scause & 0x8000000000000000) // 中断处理
    {
        // 根据 scause 的值进行分支处理
        switch (cause_code)
        {
            // 5:s mode时钟中断
        case 5:
            {
                static uint64_t last_us = 0;
                static uint32_t tick_cnt = 0;
                uint64_t now_us = get_time_us();
                if (last_us != 0)
                {
                    tick_cnt++;
                    if ((tick_cnt % 10) == 0)
                    {   // 每10次打印一次，避免刷屏影响调度
                        printf("[tick] delta=%lu us\n", (unsigned long)(now_us - last_us));
                    }
                }
                else printf("[tick] now=%lu us\n", (unsigned long)now_us);
                last_us = now_us;

                printf("[trap_handler]clock interrupt\n");
                set_next_trigger(); // 重置下一次时钟中断触发的时间
                schedule();
                break;
            }
        default:
            {
                printf("undfined interrrupt scause:%x\n", scause);
                // panic("unkonwn scause: %d\n", scause);
                break;
            }
        }
    }
    else // 异常处理
    {
        // 根据 scause 的值进行分支处理
        switch (cause_code)
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
    }

    return cx;
}

void trap_init()
{
    w_stvec((reg_t)__alltraps);
}