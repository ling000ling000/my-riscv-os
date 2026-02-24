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
PhysAddr phys_addr_from_pyhs_page_num(PhysPageNum pageNum)
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

/*---物理页帧访问---*/
// 将物理页号转换为一个字节数组的指针，以便按字节访问该物理页的内存
uint8_t* get_bytes_array(PhysPageNum phys_page_num)
{
    // 先从物理页号转换成物理地址
    PhysAddr addr = phys_addr_from_pyhs_page_num(phys_page_num);
    return (uint8_t*)addr.value;
}

// 将物理页号转换为一个页表项数组的指针，用于操作页表
PageTableEntry* get_pte_array(PhysPageNum phys_page_num)
{
    PhysAddr addr = phys_addr_from_pyhs_page_num(phys_page_num);
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
    PhysAddr addr = phys_addr_from_pyhs_page_num(ppn);
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

static StackFrameAllocator FrameAllocatorImpl;

void frame_allocator_test()
{
    StackFrameAllocator_new(&FrameAllocatorImpl);
    StackFrameAllocator_init(&FrameAllocatorImpl, \
            floor_phys(phys_addr_from_size_t(MEMORY_START)), \
            ceil_phys(phys_addr_from_size_t(MEMORY_END)));
    printk("Memoery start:%d\n",floor_phys(phys_addr_from_size_t(MEMORY_START)));
    printk("Memoery end:%d\n",ceil_phys(phys_addr_from_size_t(MEMORY_END)));
    PhysPageNum frame[10];
    for (size_t i = 0; i < 5; i++)
    {
        frame[i] = StackFrameAllocator_alloc(&FrameAllocatorImpl);
        printk("frame id:%d\n",frame[i].value);
    }
    for (size_t i = 0; i < 5; i ++ )
    {
        StackFrameAllocator_dealloc(&FrameAllocatorImpl, frame[i]);
        printk("allocator->recycled.data.value: %d\n", FrameAllocatorImpl.recycled.data[i]);
        printk("frame id:%d\n", frame[i].value);
    }
    PhysPageNum frame_test[10];
    for (size_t i = 0; i < 5; i++)
    {
        frame[i] = StackFrameAllocator_alloc(&FrameAllocatorImpl);
        printk("frame id:%d\n", frame[i].value);
    }
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

// 定义页表
typedef struct
{
    PhysPageNum root_ppn; // 根结点
    Stack frames; // 页帧
} PageTable;

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
            push(&pt->frames, frame.value); // 将新分配的物理帧号压入页表的帧栈
        }
        ppn = PageTableEntry_ppn(pte); // 取出当前PTE指向的下级页表物理页号，作为下一轮遍历的ppn
    }
    return NULL;
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
void PageTable_map(PageTable* pt, VirtPageNum vpn, PhysPageNum ppn, uint8_t pteflags)
{
    PageTableEntry* pte = find_pte_create(pt,vpn);
    assert(!PageTableEntry_is_valid(pte));
    *pte = PageTableEntry_new(ppn,PTE_V | pteflags);
}

void PageTable_unmap(PageTable* pt, VirtPageNum vpn)
{
    PageTableEntry* pte = find_pte(pt,vpn);
    assert(!PageTableEntry_is_valid(pte));
    *pte = PageTableEntry_empty();
}