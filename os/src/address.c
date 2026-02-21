#include "../include/os.h"
#include "../include/address.h"
#include "../include/stdio.h"
#include "../include/stack.h"

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
    if (allocator->recycled.top >= 1)
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
    for (size_t i = 0; i < allocator->recycled.top; i ++ )
    {
        if (ppnValue == allocator->recycled.data[i])
            return; // 页号已存在则直接回收
    }
    push(&(allocator->recycled), ppnValue);
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
