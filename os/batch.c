//
// Created by kk on 2026/2/4.
//
#include <stddef.h>
#include "os.h"
#include "context.h"
#define USER_STACK_SIZE (4096 * 2)
#define KERNEL_STACK_SIZE (4096 * 2)
#define APP_BASE_ADDRESS 0x80600000

size_t syscall(size_t id, reg_t arg1, reg_t arg2, reg_t arg3)
{
    long ret;
    asm volatile(
        /*
        * a7 放 syscall number
        * a0=a fd, a1=buf, a2=len
        * 返回值放 a0
         */
        "mv a7, %1\n\t"
        "mv a0, %2\n\t"
        "mv a1, %3\n\t"
        "mv a2, %4\n\t"
        "ecall\n\t"
        "mv %0, a0"
        : "=r" (ret)
        : "r" (id), "r" (arg1), "r" (arg2), "r" (arg3)
        : "a7", "a0", "a1", "a2", "memory"
    );
    return ret;
}

void testsys()
{
    syscall(2, 3, 4, 5);

    while (1) {}
}

uint8_t kernel_stack[KERNEL_STACK_SIZE];
uint8_t user_stack[USER_STACK_SIZE] = {0};

extern void __restore(pt_reg_t *next);

pt_reg_t tasks;

void app_init_context()
{
    // 计算用户栈的栈顶地址
    // 栈是从高地址向低地址增长的，所以栈顶是 数组首地址 + 数组大小
    reg_t user_sp = (reg_t)user_stack + USER_STACK_SIZE;
    printf("user sp: %p\n", (void*)user_sp);

    reg_t stvec = r_stvec();
    printf("stvec: 0x%lx\n", (unsigned long)stvec);

    // 初始化中断向量表，将 stvec 指向 __alltraps
    trap_init();

    // 读取当前的 sstatus 寄存器
    reg_t sstatus = r_sstatus();

    // 关键步骤：设置 sstatus 的 SPP (Supervisor Previous Privilege) 位
    // 第 8 位是 SPP，将其置为 0 表示“之前的特权级是 User 模式”
    // 这样在执行 sret 指令时，CPU 就会切换回 User 模式
    sstatus &= (0U << 8);
    w_sstatus(sstatus); // 写回sstatus
    printf("sstatus:%x\n", sstatus);

    // 设置任务的入口地址 (sepc) 为 testsys 函数的地址
    tasks.sepc = (reg_t)testsys;
    printf("tasks sepc:%x\n", tasks.sepc);

    // 设置任务的状态寄存器 (包含 SPP=User, SPIE=Enable 等)
    tasks.sstatus = sstatus;

    // 设置任务的用户栈指针
    tasks.sp = user_sp;

    // --- 在内核栈上构造 Trap 上下文 ---

    // 计算内核栈的栈顶位置，并预留出一个 pt_regs 结构体的大小
    // cx_ptr 指向内核栈顶部的这块预留空间
    pt_reg_t* cx_ptr = (pt_reg_t*)((uint8_t*)kernel_stack + KERNEL_STACK_SIZE - sizeof(pt_reg_t));
    printf("pt_regs: %d\n",sizeof(pt_reg_t));

    // 将我们准备好的任务信息填充到内核栈的这块空间中
    cx_ptr->sepc = tasks.sepc;       // 恢复后 PC 指向 testsys
    printf("cx_ptr sepc :%x\n", cx_ptr->sepc);
    printf("cx_ptr sepc adress:%x\n", &(cx_ptr->sepc));

    cx_ptr->sstatus = tasks.sstatus; // 恢复后进入 U 模式
    cx_ptr->sp = tasks.sp;           // 恢复后 SP 指向 UserStack

    // *cx_ptr = tasks[0]; // (注释代码：直接结构体复制也是可行的)
    printf("cx_ptr adress:%x\n", cx_ptr);

    // 调用汇编函数 __restore
    // 参数 cx_ptr 传递给 a0 寄存器
    // __restore 会将 cx_ptr 指向的栈上数据加载到 CPU 寄存器中，最后执行 sret
    // 此时 CPU 就像是从一次中断中返回一样，跳转到了 testsys，并切换到了 U 模式
    __restore(cx_ptr);


}