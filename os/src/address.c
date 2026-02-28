#include "../include/os.h"
#include "../include/address.h"
#include "../include/stdio.h"
#include "../include/stack.h"
#include "../include/assert.h"

/* ---地址转换--- */

// 将 uint64_t 转换为 PhysAddr，并截断高位
PhysAddr phys_addr_from_size_t(uint64_t value)
{
    PhysAddr addr;
    addr.value = value & ((1ULL << PA_WIDTH_SV39) - 1); // 保留有效的物理地址位（低56位）
    return addr;
}

// 将 uint64_t 转换为 PhysPageNum，并截断高位
PhysPageNum phys_page_num_from_size_t(uint64_t value)
{
    PhysPageNum pageNum;
    pageNum.value = value & ((1ULL << PPN_WIDTH_SV39) - 1);
    return pageNum;
}

// PhysAddr 转换回 uint64_t
uint64_t size_t_from_phys_addr(PhysAddr addr)
{
    return addr.value;
}

// PhysPageNum 转换回 uint64_t
uint64_t size_t_from_phys_page_num(PhysPageNum pageNum)
{
    return pageNum.value;
}

// 从物理页号转换成实际物理地址
PhysAddr phys_addr_from_phys_page_num(PhysPageNum pageNum)
{
    PhysAddr addr;
    addr.value = pageNum.value << PAGE_SIZE_BITS;
    return addr;
}

// u64转换为VirtAddr
VirtAddr virt_addr_from_size_t(uint64_t value)
{
    VirtAddr addr;
    addr.value = value & ((1ULL << VA_WIDTH_SV39) - 1);
    return addr;
}

// u64转换为VirtPageNum
VirtPageNum virt_page_num_from_size_t(uint64_t value)
{
    VirtPageNum pageNum;
    pageNum.value = value & ((1ULL << VPN_WIDTH_SV39) - 1);
    return pageNum;
}

// VirtAddr转换u64带符号拓展
uint64_t size_t_from_virt_addr(VirtAddr addr)
{
    // 第38位（最高有效位）为1
    if (addr.value >= (1ULL << (VA_WIDTH_SV39 - 1)))
    {
        return addr.value | ~((1ULL << VA_WIDTH_SV39) - 1); // 将高位（39以上）全部置1
    }
    else
    {
        return addr.value;
    }
}

// VirtPageNum转换u64
uint64_t size_t_from_virt_page_num(VirtPageNum pageNum)
{
    return pageNum.value;
}

/*  ---页表项操作--- */

// 根据物理页号和标志位构造一个PTE
PageTableEntry PageTableEntry_new(PhysPageNum ppn, uint8_t PTEFlags) 
{
    PageTableEntry entry;
    entry.bits = (ppn.value << 10) | PTEFlags;
    return entry;
}

//
PageTableEntry PageTableEntry_empty()
{
    PageTableEntry entry;
    entry.bits = 0;
    return entry;
}

//
PhysPageNum PageTableEntry_ppn(PageTableEntry *entry)
{
    PhysPageNum ppn;
    ppn.value = (entry->bits >> 10) & ((1UL << 44) - 1); // 右移10位去掉标志位，掩码取低44位
    return ppn;
}

// 从页表项提取标志位
uint8_t PageTableEntry_flags(PageTableEntry *entry)
{
    return entry->bits & 0xff;
}

// 判断页表项是否有效
bool PageTableEntry_is_valid(PageTableEntry *entry)
{
    uint8_t entry_flags = PageTableEntry_flags(entry);
    return (entry_flags & PTE_V) != 0;
}

/* ---物理页号计算函数--- */

// 物理地址向下取整得到物理页号
PhysPageNum floor_phys(PhysAddr phys_addr)
{
    PhysPageNum phys_page_num;
    phys_page_num.value = phys_addr.value / PAGE_SIZE;
    return phys_page_num;
}

// 物理地址向上取整到物理页号
PhysPageNum ceil_phys(PhysAddr phys_addr)
{
    PhysPageNum phys_page_num;
    phys_page_num.value = (phys_addr.value + PAGE_SIZE - 1) / PAGE_SIZE;
    return phys_page_num;
}

// 虚拟地址转换为虚拟页号
VirtPageNum virt_page_num_from_virt_addr(VirtAddr addr)
{
    VirtPageNum vpn;
    vpn.value = addr.value / PAGE_SIZE;
    return vpn;
}

/* 物理地址向下取整 */
VirtPageNum floor_virts(VirtAddr virt_addr)
{
    VirtPageNum virt_page_num;
    virt_page_num.value = virt_addr.value / PAGE_SIZE;
    return virt_page_num;
}

/*---物理页帧访问---*/
// 将物理页号转换为一个字节数组的指针，以便按字节访问该物理页的内存
uint8_t* get_bytes_array(PhysPageNum phys_page_num)
{
    // 先从物理页号转换成物理地址
    PhysAddr addr = phys_addr_from_phys_page_num(phys_page_num);
    return (uint8_t*)addr.value;
}

// 将物理页号转换为一个页表项数组的指针，用于操作页表
PageTableEntry* get_pte_array(PhysPageNum phys_page_num)
{
    PhysAddr addr = phys_addr_from_phys_page_num(phys_page_num);
    return (PageTableEntry*)addr.value;
}


/* ---分配器管理函数--- */

// 初始化分配器
void StackFrameAllocator_new(StackFrameAllocator *allocator)
{
    allocator->current = 0;
    allocator->end = 0;
    initStack(&allocator->recycled);
}

// 配置分配器的可用内存范围[l, r)
void StackFrameAllocator_init(StackFrameAllocator *allocator, PhysPageNum l, PhysPageNum r)
{
    allocator->current = l.value;
    allocator->end = r.value;
}

// 分配一个物流页帧
PhysPageNum StackFrameAllocator_alloc(StackFrameAllocator *allocator)
{
    PhysPageNum ppn;
    // printk("allocator->recycled.top: %d\n", allocator->recycled.top);

    // 优先从回收栈中分配
    if (allocator->recycled.top >= 0)
    {
        ppn.value = pop(&allocator->recycled);
    }
    else
    {
        // 回收栈空，线性分配
        if (allocator->current == allocator->end)
        {
            ppn.value = 0; // 内存耗尽
        }
        else
        {
            ppn.value = allocator->current ++;
        }
    }
    PhysAddr addr = phys_addr_from_phys_page_num(ppn);
    memset(addr.value, 0, PAGE_SIZE);
    return ppn;
}

// 回收一个物理页帧
void StackFrameAllocator_dealloc(StackFrameAllocator *allocator, PhysPageNum ppn)
{
    uint64_t ppnValue = ppn.value;

    // 安全检查，回收的页号必须小于当前已分配的指针
    if (ppnValue >= allocator->current)
    {
        printk("frame ppn=%lx has not been allocated!\n", ppnValue);
        return;
    }
    // 安全检查，防止重复回收
    // 检查未在回收列表中
    if(allocator->recycled.top>=0)
    {
        for (size_t i = 0; i < allocator->recycled.top; i ++ )
        {
            if (ppnValue == allocator->recycled.data[i])
                return; // 页号已存在则直接回收
        }
    }
    push(&(allocator->recycled), ppnValue); // 回收物理页号
}

// static StackFrameAllocator FrameAllocatorImpl;
// void frame_allocator_test()
// {
//     StackFrameAllocator_new(&FrameAllocatorImpl);
//     StackFrameAllocator_init(&FrameAllocatorImpl, \
//             floor_phys(phys_addr_from_size_t(MEMORY_START)), \
//             ceil_phys(phys_addr_from_size_t(MEMORY_END)));
//     printk("Memoery start:%d\n",floor_phys(phys_addr_from_size_t(MEMORY_START)));
//     printk("Memoery end:%d\n",ceil_phys(phys_addr_from_size_t(MEMORY_END)));
//     PhysPageNum frame[10];
//     for (size_t i = 0; i < 5; i++)
//     {
//         frame[i] = StackFrameAllocator_alloc(&FrameAllocatorImpl);
//         printk("frame id:%d\n",frame[i].value);
//     }
//     for (size_t i = 0; i < 5; i ++ )
//     {
//         StackFrameAllocator_dealloc(&FrameAllocatorImpl, frame[i]);
//         printk("allocator->recycled.data.value: %d\n", FrameAllocatorImpl.recycled.data[i]);
//         printk("frame id:%d\n", frame[i].value);
//     }
//     PhysPageNum frame_test[10];
//     for (size_t i = 0; i < 5; i++)
//     {
//         frame[i] = StackFrameAllocator_alloc(&FrameAllocatorImpl);
//         printk("frame id:%d\n", frame[i].value);
//     }
// }


StackFrameAllocator FrameAllocatorImpl; // 定义栈式帧分配器的全局实例，用于管理物理内存帧（物理页）
extern char kernelend[]; // kernelend 指向内核镜像在物理内存中加载的最后一个字节的下一个地址
// 宏定义：将地址向下对齐到页大小的整数倍
// PAGE_SIZE 通常是4KB（0x1000），PAGE_SIZE-1 是 0xFFF，~0xFFF 是页对齐掩码
// 作用：忽略页内偏移，得到该地址所在页的起始地址
#define PGROUNDDOWN(a) (((a)) & ~(PAGE_SIZE-1))
/**
 * @brief 初始化物理内存帧分配器
 * @note 该函数是操作系统启动阶段的核心内存管理初始化函数，
 *       负责划定可分配的物理内存范围并初始化分配器
 */
void frame_alloctor_init()
{
    StackFrameAllocator_new(&FrameAllocatorImpl);

    // 初始化帧分配器的内存管理范围
    StackFrameAllocator_init(
        &FrameAllocatorImpl,                     // 目标分配器实例
        ceil_phys(phys_addr_from_size_t(kernelend)),  // 起始物理地址：内核结束地址向上取整到页边界
        ceil_phys(phys_addr_from_size_t(MEMORY_END))  // 结束物理地址：系统内存上限向上取整到页边界
    );

    printk("Memory start:0x%lx\n", (uint64_t)kernelend);
    printk("Memory end:0x%lx\n", (uint64_t)MEMORY_END);
}

/* 将虚拟页号分解为三级页表索引，按照从高到低的顺序返回 */
void indexes(VirtPageNum vpn, size_t* result)
{
    size_t idx[3];
    // 从最低级（第2级）到最高级（第0级）依次提取索引
    // i=2: 第三级页表索引（最低级），i=1: 第二级，i=0: 第一级（最高级）
    for (int i = 2; i >= 0; i-- )
    {
        idx[i] = vpn.value & 0x1ff;   // 1_1111_1111 = 0x1ff
        vpn.value >>= 9; // 将虚拟页号右移9位，把下一级的9位移到最低位，为下一次提取做准备
    }

    // 将临时数组中的索引按 0→1→2 的顺序（高→低）复制到输出数组result中
    for (int i = 0; i < 3; i++ )
    {
        result[i] = idx[i];
    }
}

// 函数功能：在三级页表中查找指定虚拟页号(vpn)对应的页表项(PTE)
// 若查找路径中的页表项不存在，则自动创建（分配物理页+初始化PTE）
PageTableEntry* find_pte_create(PageTable* pt, VirtPageNum vpn)
{
    if (pt == NULL) return NULL;

    size_t idx[3]; // 储存vpn分解后的三级索引
    indexes(vpn, idx);

    PhysPageNum ppn = pt->root_ppn; // 页表根结点的物理页号，也就是三级页表的入口
    for (int i = 0; i < 3; i++ )
    {
        // 根据当前级的物理页号(ppn)和索引(idx[i])，获取对应的页表项(PTE)指针
        PageTableEntry* pte = &get_pte_array(ppn)[idx[i]];

        if (i == 2) return pte;

        // 若当前级的PTE无效（未分配下级页表），则创建新页表
        if (!PageTableEntry_is_valid(pte))
        {
            PhysPageNum frame = StackFrameAllocator_alloc(&FrameAllocatorImpl); // 分配一页物理内存（帧），用于存储下级页表
            *pte = PageTableEntry_new(frame, PTE_V); // 初始化新的PTE：关联分配的物理帧，并设置有效位(PTE_V)
            // push(&pt->frames, frame.value); // 将新分配的物理帧号压入页表的帧栈
        }
        ppn = PageTableEntry_ppn(pte); // 取出当前PTE指向的下级页表物理页号，作为下一轮遍历的ppn
    }
    return NULL;
}

// 分配一个空闲的物理页，返回其物理页号（PPN）
PhysPageNum kalloc(void)
{
    // 调用全局的栈式帧分配器（StackFrameAllocator）的 alloc 方法
    // FrameAllocatorImpl 是分配器的具体实例（全局单例）
    PhysPageNum frame =  StackFrameAllocator_alloc(&FrameAllocatorImpl);
    // 返回分配到的物理页号
    return frame;
}

// 函数功能：在三级页表中查找指定虚拟页号(vpn)对应的页表项(PTE)
// 特点：仅查找，不创建——若路径中任意一级PTE无效，直接返回NULL
PageTableEntry* find_pte(PageTable* pt, VirtPageNum vpn)
{
    if (pt == NULL) return NULL;

    size_t idx[3]; // 储存vpn分解后的三级索引
    indexes(vpn, idx);

    PhysPageNum ppn = pt->root_ppn; // 页表根结点的物理页号，也就是三级页表的入口
    for (int i = 0; i < 3; i++ )
    {
        PageTableEntry* pte = &get_pte_array(ppn)[idx[i]];
        if (i == 2) return pte;
        if (!PageTableEntry_is_valid(pte)) return NULL;
        ppn = PageTableEntry_ppn(pte);
    }
    return NULL;
}

// 将虚拟页号(vpn)映射到物理页号(ppn)，并设置页表项标志位
// void PageTable_map(PageTable* pt, VirtPageNum vpn, PhysPageNum ppn, uint8_t pteflags)
// {
//     PageTableEntry* pte = find_pte_create(pt,vpn);
//     assert(!PageTableEntry_is_valid(pte));
//     *pte = PageTableEntry_new(ppn,PTE_V | pteflags);
// }
/**
 * @brief 建立虚拟地址到物理地址的连续映射
 *
 * @param pt        指向页表结构体的指针，操作的目标页表
 * @param va        要映射的起始虚拟地址
 * @param pa        要映射的起始物理地址
 * @param size      要映射的内存大小（字节）
 * @param pteflgs   页表项（PTE）的权限标志位（如读、写、执行权限等）
 */
void PageTable_map(PageTable* pt,VirtAddr va, PhysAddr pa, uint64_t size ,uint8_t pteflgs)
{
    if(size == 0) panic("mappages: size");

    PhysPageNum ppn = floor_phys(pa); // 将物理地址向下取整，转换为物理页号（PPN），忽略页内偏移
    VirtPageNum vpn = floor_virts(va); // 将虚拟地址向下取整，转换为虚拟页号（VPN），忽略页内偏移
    uint64_t last = (va.value + size - 1) / PAGE_SIZE; // 计算映射的最后一个虚拟页号：(起始虚拟地址 + 映射大小 - 1) / 页大小

    //printk("ppn:%d\n",ppn.value);
    for(;;)
    {
        PageTableEntry* pte = find_pte_create(pt,vpn); // 在页表中查找或创建对应虚拟页号的页表项（PTE）

        assert(!PageTableEntry_is_valid(pte));
        *pte = PageTableEntry_new(ppn,PTE_V | pteflgs); // 创建新的页表项：将物理页号与权限标志位组合

        if( vpn.value == last ) // 检查是否已处理到最后一个需要映射的页
            break;
        // 一页一页映射，处理下一个虚拟页和对应的物理页
        vpn.value+=1;
        ppn.value+=1;
    }
}

void PageTable_unmap(PageTable* pt, VirtPageNum vpn)
{
    PageTableEntry* pte = find_pte(pt,vpn);
    assert(!PageTableEntry_is_valid(pte));
    *pte = PageTableEntry_empty();
}

extern char etext[]; // 由链接脚本定义，指向内核代码段（text段）的结束地址
extern char trampoline[];
/**
 * @brief 创建并初始化内核页表（Kernel Virtual Memory Make）
 * @return 初始化完成的内核页表结构体，包含页表根节点的物理页号
 * @note 该函数是内核启动阶段的核心，负责建立内核虚拟地址到物理地址的映射
 */
PageTable kvmmake(void)
{
    PageTable pt;

    // PhysPageNum root_ppn = StackFrameAllocator_alloc(&FrameAllocatorImpl);
    PhysPageNum root_ppn =  kalloc();
    pt.root_ppn = root_ppn;
    printk("root_ppn:0x%lx\n", root_ppn.value);
    printk("root_pa:0x%lx\n", phys_addr_from_phys_page_num(root_ppn).value);
    printk("etext:0x%lx\n",(uint64_t)etext);

    // 映射内核代码段
    PageTable_map(
        &pt,                                    // 目标页表：当前正在构建的内核页表
        virt_addr_from_size_t(KERNBASE),        // 起始虚拟地址：内核基地址（KERNBASE）
        phys_addr_from_size_t(KERNBASE),        // 起始物理地址：与虚拟地址相同（内核地址空间恒等映射）
        (uint64_t)etext - KERNBASE,                  // 映射大小：代码段长度（etext - 内核基地址）
        PTE_R | PTE_X                           // 页表项权限：R(读)、X(执行)
    );
    printk("finish kernel text map!\n");

    // 映射内核数据段和物理内存
    PageTable_map(
        &pt,                                    // 目标页表：当前正在构建的内核页表
        virt_addr_from_size_t((uint64_t)etext),      // 起始虚拟地址：代码段结束地址（etext）
        phys_addr_from_size_t((uint64_t)etext),      // 起始物理地址：与虚拟地址相同（恒等映射）
        MEMORY_END - (uint64_t)etext,                // 映射大小：数据段+物理内存长度（内存上限 - 代码段结束地址）
        PTE_R | PTE_W                           // 页表项权限：R(读)、W(写)
    );
    printk("finish kernel data and physical RAM map!\n");

    // 映射陷阱处理跳板（trampoline）到内核最高虚拟地址
    PageTable_map(&pt,
                  virt_addr_from_size_t(TRAMPOLINE),          // 虚拟地址（内核最高VA）
                  phys_addr_from_size_t((uint64_t)trampoline),     // 物理地址（跳板代码的实际PA）
                  PAGE_SIZE,                                  // 映射长度（一页）
                  PTE_R | PTE_X );                            // 权限
    printk("finish TRAMPOLINE Page map!\n");

    // 为每个进程分配并映射内核栈
    proc_mapstacks(&pt);
    printk("finish kernel stack map!\n");

    return pt;
}

PageTable kernel_pagetable; // 内核页表结构体
uint64_t kernel_satp;       // 当前内核页表对应的 SATP token
/**
 * @brief 初始化内核页表（构建页表结构，但尚未启用分页）
 * @note 该函数仅创建页表映射关系，不修改硬件寄存器，属于“准备阶段”
 */
void kvminit()
{
    kernel_pagetable = kvmmake();
}

/**
 * @brief 在当前硬件核（hart）上启用内核页表（真正开启分页机制）
 * @note 该函数是硬件层面的分页启用操作，针对RISC-V架构的SATP寄存器和TLB操作
 */
void kvminithart()
{
    kernel_satp = MAKE_SATP(kernel_pagetable.root_ppn.value);
    printk("satp1:%lx\n", kernel_satp);
    sfence_vma(); // sfence_vma：RISC-V的TLB刷新指令，清空当前核的TLB缓存，避免旧的页表项生效
    w_satp(kernel_satp); // w_satp：RISC-V写SATP寄存器的汇编封装函数，写入后分页机制立即生效
    sfence_vma(); // 刷新TLB中过时的条目，确保新页表生效
    reg_t satp = r_satp(); // 读取SATP寄存器的值并保存
    printk("satp2:%lx\n", satp);
}
