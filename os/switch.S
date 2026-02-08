.altmacro                # 启用备用宏模式，允许使用 %n 这样的参数符号
.macro SAVE_SN n         # 定义宏 SAVE_SN，参数为 n
    sd s\n, (\n+2)*8(a0) # 将 s[n] 寄存器保存到地址 a0 + (n+2)*8 处
                         # 为什么是 n+2？因为偏移量 0 是 ra，8 是 sp，s0 从 16 (即 2*8) 开始
.endm

.macro LOAD_SN n         # 定义宏 LOAD_SN，参数为 n
    ld s\n, (\n+2)*8(a1) # 从地址 a1 + (n+2)*8 处加载数据到 s[n] 寄存器
.endm

.section .text       # 指定代码段
    .globl __switch      # 导出符号，使 C 语言可以调用
__switch:
    # 阶段 [1]：参数约定
    # 根据 RISC-V 调用规范 (Calling Convention)：
    # a0 寄存器保存第一个参数：current_task_cx_ptr (当前任务上下文指针)
    # a1 寄存器保存第二个参数：next_task_cx_ptr    (下一个任务上下文指针)
    # __switch(
        #     current_task_cx_ptr: *mut TaskContext,
        #     next_task_cx_ptr: *const TaskContext
        # )

    # 阶段 [2]
    # save kernel stack of current task
    sd sp, 8(a0)        # 将当前栈指针 sp 保存到结构体偏移 8 的位置

    # save ra & s0~s11 of current execution
    sd ra, 0(a0)        # 将返回地址 ra 保存到结构体偏移 0 的位置
                        # 重要：这里的 ra 指向调用 __switch 的那行代码的下一行（通常在 schedule 函数中）

    .set n, 0           # 设置宏变量 n = 0
    .rept 12            # 循环 12 次 (对应 s0 到 s11)
        SAVE_SN %n      # 调用宏，生成 sd s0, 16(a0); sd s1, 24(a0)... 等指令
        .set n, n + 1   # n 自增
    .endr

    # 阶段 [3]
    # restore ra & s0~s11 of next execution
    ld ra, 0(a1)        # 从下一个任务的结构体偏移 0 处加载返回地址到 ra
                        # 这个 ra 是该任务上次被挂起时保存的地址（或者是新任务的入口地址）

    .set n, 0           # 重置 n = 0
    .rept 12            # 循环 12 次
        LOAD_SN %n      # 调用宏，生成 ld s0, 16(a1); ld s1, 24(a1)... 等指令
        .set n, n + 1   # n 自增
    .endr

    # restore kernel stack of next task
    ld sp, 8(a1)        # 从下一个任务的结构体偏移 8 处加载栈指针到 sp
                        # 【关键点】：执行完这行代码，CPU 的栈就已经切换到了新任务的内核栈！

    # 阶段 [4]
    ret                 # return 指令，等同于 jalr x0, 0(ra)