.global __alltraps
.align 4
__alltraps:
    # --- 步骤 1：栈指针切换 ---
    # csrrw (CSR Read/Write): 原子交换指令
    # 操作前：sp 指向用户栈，sscratch 指向内核栈（由内核预先设定）
    # 操作后：sp 指向内核栈，sscratch 保存了用户栈指针
    csrrw sp, sscratch, sp

    # --- 步骤 2：在内核栈上开辟空间 ---
    # sp 向低地址移动，开辟 34 * 8 字节的空间
    # 34 个位置包括：32 个通用寄存器 + sstatus + sepc
    addi sp, sp, -34*8

    # --- 步骤 3：保存通用寄存器 (x1-x31) ---
    # sd (Store Double word): 将 64 位寄存器值写入栈内存
    sd x1, 1*8(sp)    # 保存 ra (返回地址)
    # 跳过 x2 (sp)，因为此时 sp 寄存器已经指向内核栈，我们需要保存的是用户栈指针
    sd x3, 3*8(sp)

    sd x4, 4*8(sp) # 保存 tp (线程指针)

    # 依次保存剩余通用寄存器 x5 至 x31
    sd x5, 5*8(sp)
    sd x6, 6*8(sp)
    sd x7, 7*8(sp)
    sd x8, 8*8(sp)
    sd x9, 9*8(sp)
    sd x10,10*8(sp)
    sd x11, 11*8(sp)
    sd x12, 12*8(sp)
    sd x13, 13*8(sp)
    sd x14, 14*8(sp)
    sd x15, 15*8(sp)
    sd x16, 16*8(sp)
    sd x17, 17*8(sp)
    sd x18, 18*8(sp)
    sd x19, 19*8(sp)
    sd x20, 20*8(sp)
    sd x21, 21*8(sp)
    sd x22, 22*8(sp)
    sd x23, 23*8(sp)
    sd x24, 24*8(sp)
    sd x25, 25*8(sp)
    sd x26, 26*8(sp)
    sd x27, 27*8(sp)
    sd x28, 28*8(sp)
    sd x29, 29*8(sp)
    sd x30, 30*8(sp)
    sd x31, 31*8(sp)

    # --- 步骤 4：保存特殊CSR和用户栈指针 ---
    # 此时t0/t1/t2已被保存到栈上，可以临时用作暂存寄存器
    csrr t0, sstatus  # 读取sstatus(包含中断使能状态和特权级信息)
    csrr t1, sepc     # 读取sepc(记录了发生 Trap 时的指令地址)
    sd t0, 32*8(sp)   # 将sstatus保存到结构体指定位置
    sd t1, 33*8(sp)   # 将sepc保存到结构体指定位置

    # 读取 sscratch，根据第一步的交换，这里面存放的是“用户态原本的 sp”
    csrr t2, sscratch
    sd t2, 2*8(sp)    # 将用户栈指针保存到结构体的x2 (sp)位置

    # --- 步骤 5：调用 C 语言处理函数 ---
    # set input argument of trap_handler(cx: &mut TrapContext)
    mv a0, sp         # 将当前 sp (即 TrapContext 结构体的起始地址) 赋值给 a0
                      # a0 是 RISC-V 调用约定的第一个参数寄存器
    call trap_handler # 跳转执行 C 语言编写的 trap_handler 函数
    mv sp, a0
    j __restore

.global __restore
.align 4
__restore:
    # case1: start running app by __restore (启动新任务)
    # case2: back to U after handling trap (处理完中断返回)
    # 此时 a0 寄存器持有 TrapContext 的指针（可能由 trap_handler 返回）
    # mv sp, a0  # 将 sp 指向 TrapContext 的位置

    # 注意：此时 sscratch 的状态并不确定，需要通过恢复过程重置

    # --- 步骤 1：恢复 CSR 状态 ---
    ld t0, 32*8(sp) # 从栈加载到原sstatus
    ld t1, 33*8(sp) # 从栈加载到原sepc
    ld t2, 2*8(sp) # 从栈加载原用户栈指针

    csrw sstatus, t0 # 写入sstatus寄存器
    csrw sepc, t1 # 写入sepc寄存器，设定sret返回后到跳转地址
    csrw sscratch, t2 # 关键操作：将用户栈指针放回 sscratch
                      # 这是为了配合 __alltraps 开头的 csrrw 指令

    # --- 步骤 2：恢复通用寄存器 ---
    # restore general-purpuse registers except sp/tp
    # 依次从栈恢复 x1, x3-x31。x2 (sp) 必须最后处理，否则会丢失内核栈指针
    ld x1, 1*8(sp)
    ld x3, 3*8(sp)
    ld x4, 4*8(sp)
    ld x5, 5*8(sp)
    ld x6, 6*8(sp)
    ld x7, 7*8(sp)
    ld x8, 8*8(sp)
    ld x9, 9*8(sp)
    ld x10,10*8(sp)
    ld x11, 11*8(sp)
    ld x12, 12*8(sp)
    ld x13, 13*8(sp)
    ld x14, 14*8(sp)
    ld x15, 15*8(sp)
    ld x16, 16*8(sp)
    ld x17, 17*8(sp)
    ld x18, 18*8(sp)
    ld x19, 19*8(sp)
    ld x20, 20*8(sp)
    ld x21, 21*8(sp)
    ld x22, 22*8(sp)
    ld x23, 23*8(sp)
    ld x24, 24*8(sp)
    ld x25, 25*8(sp)
    ld x26, 26*8(sp)
    ld x27, 27*8(sp)
    ld x28, 28*8(sp)
    ld x29, 29*8(sp)
    ld x30, 30*8(sp)
    ld x31, 31*8(sp)

    # --- 步骤 3：释放栈空间并切换栈 ---
    # 此时 sp 仍指向内核栈，需回收之前分配的 34*8 字节空间
    addi sp, sp, 34*8

    # 再次执行原子交换：
    # 操作前：sp 指向内核栈（栈底），sscratch 指向用户栈
    # 操作后：sp 指向用户栈，sscratch 指向内核栈（为下一次 Trap 做准备）
    csrrw sp, sscratch, sp

    # --- 步骤 4：特权级返回 ---
    # sret (Supervisor Return):
    # 1. PC 跳转到 sepc 寄存器的值
    # 2. 特权级切换回 sstatus.SPP 指定的模式 (通常是 User 模式)
    # 3. 恢复 sstatus.SPIE 中断使能位
    sret