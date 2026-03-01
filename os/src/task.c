#include "../include/os.h"

// 记录当前正在运行的任务索引（ID）
static int _current = 0;
// 记录当前已创建的任务总数，指向栈顶
static int _top = 0;
// 分配pid
int next_pid = 1;

// 定义内核栈数组，二维数组，每个任务拥有独立的内核栈空间
uint8_t KernelStack[MAX_TASKS][KERNEL_STACK_SIZE];
// 定义用户栈数组，每个任务拥有独立的用户栈空间
uint8_t UserStack[MAX_TASKS][USER_STACK_SIZE];

// 定义任务控制块数组，用于存放所有任务的管理信息
struct TaskControlBlock tasks[MAX_TASKS];
extern PageTable kernel_pagetable;

// 兼容模式使用单一内核页表运行全部任务，fork 时必须给子进程单独栈页，避免父子共享同一用户栈。
static int fork_setup_child_stack_compat(struct TaskControlBlock *p, struct TaskControlBlock *np, pt_reg_t *cx_ptr)
{
    uint64_t parent_stack_va = p->ustack - PAGE_SIZE;
    uint64_t parent_stack_top = p->ustack;
    VirtPageNum parent_stack_vpn = floor_virts(virt_addr_from_size_t(parent_stack_va));
    PageTableEntry *parent_stack_pte = find_pte(&p->page_table, parent_stack_vpn);
    if (parent_stack_pte == 0 || !PageTableEntry_is_valid(parent_stack_pte))
        return -1;

    uint64_t parent_stack_pa = PTE2PA(parent_stack_pte->bits);

    PhysPageNum child_stack_ppn = kalloc();
    if (child_stack_ppn.value == 0)
        return -1;
    uint64_t child_stack_pa = phys_addr_from_phys_page_num(child_stack_ppn).value;

    uint64_t child_stack_top = 0x40000000UL + ((uint64_t)np->pid + 1) * USER_STACK_SIZE;
    uint64_t child_stack_va = child_stack_top - PAGE_SIZE;

    PageTable_map(&np->page_table,
                  virt_addr_from_size_t(child_stack_va),
                  phys_addr_from_size_t(child_stack_pa),
                  PAGE_SIZE,
                  PTE_R | PTE_W | PTE_U);

    PageTable_map(&kernel_pagetable,
                  virt_addr_from_size_t(child_stack_va),
                  phys_addr_from_size_t(child_stack_pa),
                  PAGE_SIZE,
                  PTE_R | PTE_W | PTE_U);

    memcpy((void *)child_stack_pa, (void *)parent_stack_pa, PAGE_SIZE);

    long long delta = (long long)child_stack_top - (long long)p->ustack;
    uint64_t *stack_words = (uint64_t *)child_stack_pa;
    for (int i = 0; i < PAGE_SIZE / (int)sizeof(uint64_t); i++)
    {
        uint64_t v = stack_words[i];
        if (v >= parent_stack_va && v <= parent_stack_top)
            stack_words[i] = (uint64_t)((long long)v + delta);
    }

    reg_t *regs = &cx_ptr->x0;
    for (int i = 0; i < 32; i++)
    {
        if (regs[i] >= parent_stack_va && regs[i] <= parent_stack_top)
            regs[i] = (reg_t)((long long)regs[i] + delta);
    }

    np->ustack = child_stack_top;
    if (np->base_size < child_stack_top)
        np->base_size = child_stack_top;
    return 0;
}

// 初始化任务上下文结构体 TaskContext
// 参数 kstack_ptr：该任务内核栈的栈顶指针（指向 TrapContext）
struct TaskContext tcx_init(reg_t kstack_ptr)
{
    struct TaskContext task_ctx;

    // 核心设置：将返回地址 ra 设置为 __restore 函数的入口
    // 任务调度切换完成后，CPU 会跳转到 __restore 执行现场恢复
    // task_ctx.ra = __restore;
    task_ctx.ra = trap_return;

    // 设置栈指针 sp，指向内核栈顶保存的 TrapContext 位置
    task_ctx.sp = kstack_ptr;

    // 初始化被调用者保存寄存器（Callee-Saved Registers）s0 到 s11
    // 新任务没有历史状态，将其全部清零
    task_ctx.s0 = 0;
    task_ctx.s1 = 0;
    task_ctx.s2 = 0;
    task_ctx.s3 = 0;
    task_ctx.s4 = 0;
    task_ctx.s5 = 0;
    task_ctx.s6 = 0;
    task_ctx.s7 = 0;
    task_ctx.s8 = 0;
    task_ctx.s9 = 0;
    task_ctx.s10 = 0;
    task_ctx.s11 = 0;

    return task_ctx; // 返回初始化完成的上下文结构
}

/*
 * 为每个应用程序映射内核栈，内核地址空间已提前完成基础映射（如页表根节点、代码/数据段）
 * kpgtbl：内核页表指针，所有映射都会写入这个页表
 */
void proc_mapstacks(PageTable* kpgtbl)
{
    // 定义任务控制块指针，遍历所有进程
    struct TaskControlBlock *p;

    // 遍历所有任务（从tasks数组起始到MAX_TASKS结束）
    for(p = tasks; p < &tasks[MAX_TASKS]; p++)
    {
        // 1. 分配一个物理页作为该进程的内核栈物理内存
        // kalloc()返回物理页号 → 转换为物理地址（PA）→ 转为char*类型
        char *pa = (char*)phys_addr_from_phys_page_num(kalloc()).value;
        // 检查分配是否失败（pa=0表示无空闲物理页）
        if(pa == 0)
            panic("kalloc");  // 分配失败则内核崩溃（panic）

        // 2. 计算该进程内核栈的虚拟地址（VA）
        // KSTACK是宏，根据进程索引（p - tasks）计算专属的虚拟地址
        uint64_t va = KSTACK((int) (p - tasks));

        // 3. 将内核栈的虚拟地址映射到物理地址（写入内核页表）
        PageTable_map(
          kpgtbl,                          // 目标页表（内核页表）
          virt_addr_from_size_t(va),  // 虚拟起始地址（栈底）
          phys_addr_from_size_t((uint64_t)pa),  // 物理起始地址
          PAGE_SIZE,                       // 映射长度（一页，4KB）
          PTE_R | PTE_W                    // 权限：可读可写，禁止执行
        );

        // 4. 记录该进程的内核栈顶地址到TCB中
        // va + PAGE_SIZE 是栈底，va + 2*PAGE_SIZE 是栈顶（栈向低地址生长）
        p->kstack = va + PAGE_SIZE;
    }
}

// 为每个应用程序分配一页物理内存用于存储陷阱上下文
void proc_trap(struct TaskControlBlock* p)
{
    p->trap_cx_ppn = phys_addr_from_phys_page_num(kalloc()).value; // 为每个程序分配一页trap物理内存
    printk("trap value : %p\n", p->trap_cx_ppn);
    memset(&p->task_context, 0, sizeof(p->task_context)); // 初始化任务上下文全部为0
}

extern char trampoline[];
/* 为用户程序创建页表，映射跳板页和trap上下文页*/
void proc_pagetable(struct TaskControlBlock* p)
{
    PageTable page_table;
    page_table.root_ppn = kalloc();

    // 映射跳板页到用户虚拟地址空间
    PageTable_map(&page_table,
               virt_addr_from_size_t(TRAMPOLINE),
              phys_addr_from_size_t((uint64_t)trampoline),
             PAGE_SIZE ,
           PTE_R | PTE_X
               );
    printk("finish user TRAMPOLINE map!\n");

    // 映射用户程序的trap上下文页到用户虚拟地址空间
    PageTable_map(&page_table,
               virt_addr_from_size_t(TRAPFRAME),
              phys_addr_from_size_t(p->trap_cx_ppn),
             PAGE_SIZE,
           PTE_R | PTE_W );
    printk("finish user TRAPFRAME map!\n");

    p->page_table = page_table;
    printk("p->pagetable:%p\n",p->page_table.root_ppn.value);
}

// 初始化全局进程控制块数组
void proc_init()
{
    struct TaskControlBlock* pcb;
    for (pcb = tasks; pcb < &tasks[MAX_TASKS]; pcb ++)
    {
        pcb->task_state = UnInit;
    }
}


// 为指定ID的应用程序创建进程控制块（TCB），并初始化陷阱上下文和页表
TaskControlBlock* task_create_pt(size_t app_id)
{
    if (_top < MAX_TASKS)
    {
        proc_trap(&tasks[app_id]); // 为应用程序分配一页物理内存，用于存储陷阱上下文,每个进程独立
        proc_pagetable(&tasks[app_id]); // 为应用程序创建独立的用户页表，每个进程有专属页表，实现地址空间隔离
        _top ++;
    }
    return &tasks[app_id];
}

/* 返回当前执行的应用程序的trap上下文的地址 */
uint64_t get_current_trap_cx()
{
    return tasks[_current].trap_cx_ppn;
}

// 返回当前用户进程的页表token
uint64_t current_user_token()
{
    extern uint64_t kernel_satp;
    return kernel_satp;
}

extern uint64_t kernel_satp; // satp寄存器值
void app_init(size_t app_id)
{
    pt_reg_t* cx_ptr = tasks[app_id].trap_cx_ppn;
    reg_t sstatus = r_sstatus();
    // 仅构造用户上下文的 sstatus，不修改当前内核正在运行的 CSR。
    sstatus &= ~(1UL << 8); // SPP=0, sret 回到 U
    sstatus |= (1UL << 5);  // SPIE=1, 进入 U 后允许后续时钟中断

    cx_ptr->sepc = tasks[app_id].entry; // 设置程序入口
    printk("cx_ptr->sepc:%p\n",cx_ptr->sepc);
    cx_ptr->sstatus = sstatus;

    cx_ptr->sp = (reg_t)tasks[app_id].ustack; // 设置用户栈虚拟地址
    printk("cx_ptr->sp:%p\n",cx_ptr->sp);

    cx_ptr->kernel_satp = kernel_satp; // 内核页表token
    cx_ptr->kernel_sp = tasks[app_id].kstack; // 内核栈虚拟地址
    printk("cx_ptr->kernel_sp:%p\n",cx_ptr->kernel_sp);

    cx_ptr->trap_handler = (uint64_t)trap_handler; // trap handler地址
    printk("cx_ptr->trap_handler:%p\n",cx_ptr->trap_handler);

    /* 构造每个任务任务控制块中的任务上下文，设置 ra 寄存器为 trap_return 的入口地址*/
    tasks[app_id].task_context = tcx_init((reg_t)tasks[app_id].kstack);
    tasks[app_id].task_state = Ready;
    tasks[app_id].pid = alloc_pid(); // 分配pid
}

// 创建新任务
// 参数 task_entry：指向用户程序入口函数的指针
void task_create(void (*task_entry)(void))
{
    // 检查当前任务数是否未达到上限
    if (_top  < MAX_TASKS)
    {
        /* 计算 TrapContext 在内核栈顶的存储位置
           此处指针运算意为：内核栈基址 + 栈大小 - TrapContext 结构体大小 */
        /* 1) 计算该任务内核栈顶，并在栈顶预留 pt_reg_t 作为 Trap 上下文 */
        uint8_t *kbase = (uint8_t *)&KernelStack[_top][0];
        uint8_t *ktop  = kbase + KERNEL_STACK_SIZE;
        pt_reg_t *cx_ptr = (pt_reg_t *)(ktop - sizeof(pt_reg_t));

        // 计算用户栈顶地址（栈向下增长，故为基址 + 大小）
        uint8_t *ubase = (uint8_t *)&UserStack[_top][0];
        reg_t user_sp  = (reg_t)(ubase + USER_STACK_SIZE);

        // 读取当前 sstatus (Supervisor Status) 寄存器的值
        reg_t sstatus = r_sstatus();

        // 修改 sstatus 的 SPP 位（第 8 位）
        // 将其置为 0，表示中断返回（sret）后特权级切换至 User 模式
        // sstatus &= ~(1UL << 8);
        // sstatus |= (1UL << 8);
        sstatus &= ~(1UL << 8); // SPP=0, sret 回到 U


        // 将修改后的状态写回 sstatus 寄存器（这一步在逻辑上主要用于下方赋值）
        // w_sstatus(sstatus);

        /* 初始化内核栈顶的 TrapContext */
        cx_ptr->sepc = (reg_t)task_entry; // 设置 sepc (Exception PC)，sret 后 CPU 将跳转至 task_entry 执行
        cx_ptr->sstatus = sstatus; // 保存构造好的 sstatus，确保特权级正确切换
        cx_ptr->sp = (reg_t)user_sp; // 设置用户栈指针，进入用户态后 sp 寄存器将使用此值
        // cx_ptr->sscratch = (reg_t)user_sp;   // 这里保存用户栈指针


        printf("[task_create] id=%d sepc=%lx user_sp=%lx kcx=%lx\n", _top, cx_ptr->sepc, cx_ptr->sp, (reg_t)cx_ptr);

        /* 初始化任务控制块中的任务上下文 (TaskContext)
          调用 tcx_init，传入 TrapContext 的地址作为内核栈指针 */
        tasks[_top].task_context = tcx_init((reg_t)cx_ptr);
        // 将任务状态标记为 Ready（就绪），等待调度器选中
        tasks[_top].task_state = Ready;

        // 任务计数加 1
        _top ++;
    }
}

void schedule()
{
    // 如果没有任何任务，触发内核恐慌 (Panic)
    if (_top <= 0)
    {
        panic("Num of task should be greater than zero!\n");
        return;
    }

    int current = _current;
    int next = -1;

    // 轮询寻找可运行任务：
    // 1) 优先 Ready
    // 2) 容错：若某个“非当前任务”误标成 Running，也允许切过去恢复轮转
    for (int i = 1; i <= _top; i++)
    {
        int idx = (current + i) % _top;
        TaskState st = tasks[idx].task_state;
        if (st == Ready || (idx != current && st == Running))
        {
            next = idx;
            break;
        }
    }

    if (next < 0 || next == current)
        return;

    TaskContext *current_task_cx_ptr = &(tasks[current].task_context);
    TaskContext *next_task_cx_ptr = &(tasks[next].task_context);

    if (tasks[current].task_state == Running)
        tasks[current].task_state = Ready;
    tasks[next].task_state = Running;
    _current = next;

    // 调用汇编函数 __switch 执行上下文切换
    __switch(current_task_cx_ptr, next_task_cx_ptr);
}

// 启动第一个任务（仅在系统初始化阶段调用一次）
void run_first_task()
{
    // 将第 0 号任务标记为运行状态
    tasks[0].task_state = Running;

    // 获取第 0 号任务的上下文指针
    TaskContext *next_task_cx_ptr = &(tasks[0].task_context);

    // 创建一个未使用的临时上下文，仅为了满足 __switch 的参数要求
    // 因为当前没有“上一个任务”需要保存
    TaskContext _unused;

    // 执行切换：加载 next_task_cx_ptr 中的内容并跳转
    // 这将加载 ra=__restore，最终通过 sret 进入任务 0 的用户代码
    printk("[os] switching to first task: cx=%lx ra=%lx\n",
       (reg_t)&tasks[0].task_context, (reg_t)tasks[0].task_context.ra);
    printk("[os] first task ksp(trapframe)=%lx\n", (reg_t)tasks[0].task_context.sp);
    __switch(&_unused, next_task_cx_ptr);

    // 如果 __switch 返回，说明系统逻辑出现严重错误
    panic("unreachable in run_first_task!");
}

// 分配pid
int alloc_pid()
{
    int pid;
    pid = next_pid;
    next_pid ++;
    return pid;
}

// 进程控制块（PCB）的分配与初始化
struct TaskControlBlock* alloc_proc()
{
    struct TaskControlBlock* pcb;

    // 遍历全部任务
    for (pcb = tasks; pcb < &tasks[MAX_TASKS]; pcb ++)
    {
        // 查找tasks里的空闲槽位
        if (pcb->task_state == UnInit)
            goto found; // 找到空位，跳转到初始化代码块
    }
    return 0;

found:
    pcb->pid = alloc_pid(); // 分配进程id
    pcb->task_state = Ready; // 修改进程状态
    proc_trap(pcb); // 为进程分配一页内存存放trap上下文
    proc_pagetable(pcb); // 创建用户程序的页表
    return pcb;
}

int __sys_fork()
{
    struct TaskControlBlock* np; // 子进程
    struct TaskControlBlock* p; // 父进程

    p = &tasks[_current];

    // 分配进程槽位，alloc_proc 会寻找一个状态为 UnInit 的 PCB 并初始化基础字段
    if ((np = alloc_proc()) == 0)
        return -1;

    // 复制trap上下文
    memcpy((void*)np->trap_cx_ppn, (void*)p->trap_cx_ppn, PAGE_SIZE);

    // 获取子进程 Trap 上下文的指针
    pt_reg_t* cx_ptr = (pt_reg_t*)np->trap_cx_ppn;

    // a0 是函数返回值寄存器, 子进程被调度运行时，会恢复这个 Trap 上下文，a0=0，因此用户态看到的返回值就是 0
    cx_ptr->a0 = 0;
    cx_ptr->kernel_sp = np->kstack;

    // 复制tcb数据
    np->entry = p->entry;
    np->base_size = p->base_size;
    np->parent = p;
    np->ustack = p->ustack;

    if (fork_setup_child_stack_compat(p, np, cx_ptr) < 0)
        return -1;

    // ra (返回地址): 设置为 trap_return。
    // 当子进程第一次被调度器选中运行时，__switch 会跳转到 trap_return，
    // 从而恢复 Trap 上下文，最终通过 sret 返回用户态。
    np->task_context.ra = trap_return;

    // sp (栈指针): 设置为子进程的内核栈顶。
    // 这样子进程就有了独立的内核栈空间。
    np->task_context.sp = np->kstack;

    _top++; // 更新进程计数（可能是全局变量，记录活跃进程数）
    return np->pid; // 父进程返回子进程的 PID
}
