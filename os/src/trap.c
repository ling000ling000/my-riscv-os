#include "../include/os.h"
#include "../include/context.h"
#include "../include/riscv.h"

// 声明外部符号 __alltraps
// 这个符号在汇编文件 (entry.S) 中定义，是所有中断/异常的统一汇编入口
extern void __alltraps(void);
extern void __restore(pt_reg_t *next);

void trap_from_kernel()
{
    panic("a trap from kernel!\n");
}

void set_kernel_trap_entry()
{
    w_stvec((reg_t)trap_from_kernel); // 写入stvec寄存器，参数为trap_from_kernel的地址
}

void set_user_trap_entry()
{
    w_stvec((reg_t)TRAMPOLINE); // TRAMPOLINE是跳板页的虚拟地址常量
}

/* Trap 处理主函数
 */
void trap_handler()
{
    set_kernel_trap_entry();
    pt_reg_t* cx = get_current_trap_cx();

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
                set_next_trigger();               // 设置下一次定时器中断的触发时间
                schedule();                       // 执行进程调度，切换到下一个就绪进程
                break;
            }
        default:
            {
                printk("undfined interrrupt scause:%x\n", scause);
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
                cx->a0 = __SYSCALL(cx->a7, cx->a0, cx->a1, cx->a2);
                break;
            }
        default:
            {
                printk("scause=%lx sepc=%lx stval=%lx sstatus=%lx\n", r_scause(), r_sepc(), r_stval(), r_sstatus());
                panic("unkonwn scause: %d\n", scause);
                break;
            }
        }
    }

    trap_return();
}

void trap_return()
{
    /* 把 stvec 设置为内核和应用地址空间共享的跳板页面的起始地址 */
    set_user_trap_entry();
    /* 当前任务 Trap 上下文地址：隔离模式使用用户虚拟地址 TRAPFRAME，兼容模式使用内核可直访物理地址。 */
#if ENABLE_PER_TASK_SATP
    uint64_t trap_cx_ptr = TRAPFRAME;
#else
    uint64_t trap_cx_ptr = get_current_trap_cx();
#endif
    /* 要继续执行的应用地址空间的 token */
    uint64_t user_satp = current_user_token();
    // 计算__restore函数在跳板页中的绝对虚拟地址
    uint64_t restore_va = (uint64_t)__restore - (uint64_t)__alltraps + TRAMPOLINE;

    // printk("trap_cx_ptr:%p\n",trap_cx_ptr);
    // printk("user_satp:%p\n",user_satp);
    // printk("restore_va:%p\n",restore_va);

    asm volatile (
            "fence.i\n\t"
            "mv a0, %0\n\t"  // 将trap_cx_ptr传递给a0寄存器
            "mv a1, %1\n\t"  // 将user_satp传递给a1寄存器
            "jr %2\n\t"      // 跳转到restore_va的位置执行代码
            :
            : "r" (trap_cx_ptr),
            "r" (user_satp),
            "r" (restore_va)
            : "a0", "a1"
        );
}

void trap_init()
{
    w_stvec((reg_t)__alltraps);
}
