#ifndef MY_RISCV_OS_ADDRESS_H
#define MY_RISCV_OS_ADDRESS_H

#include "types.h"
#include "stack.h"

// 页表权限位掩码
#define PTE_V (1 << 0) // 有效位
#define PTE_R (1 << 1) // 可读权限
#define PTE_W (1 << 2) // 可写
#define PTE_X (1 << 3) // 可执行
#define PTE_U (1 << 4) // 用户态可访问
#define PTE_G (1 << 5) // 全局映射
#define PTE_A (1 << 6) // 访问位
#define PTE_D (1 << 7) // 脏位

#define PAGE_SIZE 0x1000 // 一页大小4b
#define PAGE_SIZE_BITS 0xc // 页内偏移12

#define PA_WIDTH_SV39 56 // 物理地址长度
#define VA_WIDTH_SV39 39 // 虚拟地址长度
#define PPN_WIDTH_SV39 (PA_WIDTH_SV39 - PAGE_SIZE_BITS) // 物理页号宽度56-12=44位
#define VPN_WIDTH_SV39 (VA_WIDTH_SV39 - PAGE_SIZE_BITS) // 虚拟页号宽度39-12=27位

#define MEMORY_END 0x80800000 // 可用内存结束地址
#define MEMORY_START 0x80200000 // 可用内存起始地址

//内核开始地址
#define KERNBASE 0x80200000L

// Sv39的最大地址空间 512G
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

//跳板页开始位置
#define TRAMPOLINE (MAXVA - PAGE_SIZE)

//计算应用内核栈的地址，每个应用的内核栈下都有一个无效的守卫页
#define KSTACK(p) (TRAMPOLINE - ((p)+1)* 2*PAGE_SIZE)

//Trap页开始位置
#define TRAPFRAME (TRAMPOLINE - PAGE_SIZE)

#define PGROUNDUP(sz)  (((sz)+PAGE_SIZE-1) & ~(PAGE_SIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PAGE_SIZE-1))

#define MAKE_PAGETABLE(satp) ( satp & (SATP_SV39 - 1) )

// 宏定义：SATP寄存器的SV39模式标识位
#define SATP_SV39 (8ULL << 60)

// 宏定义：构造SATP寄存器的值
// SATP_SV39：设置分页模式为SV39
// pagetable：页表根节点的物理页号（PPN），占据SATP寄存器的低44位（对于SV39）
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64_t)pagetable)))

#define PTE2PA(pte) (((pte) >> 10) << 12) // 从页表项中提取物理地址
#define PTE_FLAGS(pte) ((pte) & 0x3FF) // 提取权限标志位

// 物理地址
typedef struct
{
    uint64_t value;
} PhysAddr;

// 虚拟地址
typedef struct
{
    uint64_t value;
} VirtAddr;

// 物理页号
typedef struct
{
    uint64_t value;
} PhysPageNum;

// 虚拟页号
typedef struct
{
    uint64_t value;
} VirtPageNum;

// 页表结构体
typedef struct
{
    uint64_t bits;
} PageTableEntry;

// 栈式帧分配器结构体
typedef struct
{
    uint64_t current; // 空闲内存起始物理页号
    uint64_t end; // 空闲内存结束物理页号
    Stack recycled; // 回收栈
} StackFrameAllocator;

// 定义页表
typedef struct
{
    PhysPageNum root_ppn; // 根结点
    // Stack frames; // 页帧
} PageTable;

// 地址转换
PhysAddr phys_addr_from_size_t(uint64_t v);
PhysPageNum phys_page_num_from_size_t(uint64_t v);
PhysAddr phys_addr_from_phys_page_num(PhysPageNum pageNum);
uint64_t size_t_from_phys_addr(PhysAddr v);
uint64_t size_t_from_phys_page_num(PhysPageNum v);
VirtAddr virt_addr_from_size_t(uint64_t v);
VirtPageNum virt_page_num_from_size_t(uint64_t v);
uint64_t size_t_from_virt_addr(VirtAddr v);
uint64_t size_t_from_virt_page_num(VirtPageNum v);

// 页表项操作
PageTableEntry PageTableEntry_new(PhysPageNum ppn, uint8_t PTEFlags);
PageTableEntry PageTableEntry_empty();
PhysPageNum PageTableEntry_ppn(PageTableEntry *entry);
uint8_t PageTableEntry_flags(PageTableEntry *entry);
bool PageTableEntry_is_valid(PageTableEntry *entry);

// 物理页计算
PhysPageNum floor_phys(PhysAddr phys_addr);
PhysPageNum ceil_phys(PhysAddr phys_addr);

// 分配器操作
void StackFrameAllocator_new(StackFrameAllocator* allocator);
void StackFrameAllocator_init(StackFrameAllocator *allocator, PhysPageNum l, PhysPageNum r);
PhysPageNum StackFrameAllocator_alloc(StackFrameAllocator *allocator);
void StackFrameAllocator_dealloc(StackFrameAllocator *allocator, PhysPageNum ppn);
extern void frame_allocator_test();

extern void frame_alloctor_init();
extern void kvminit();
extern void kvminithart();
PhysPageNum kalloc(void);
void PageTable_map(PageTable* pt,VirtAddr va, PhysAddr pa, uint64_t size ,uint8_t pteflgs);

VirtPageNum floor_virts(VirtAddr virt_addr);
PageTableEntry* find_pte(PageTable* pt, VirtPageNum vpn);
int uvmcopy(PageTable* old, PageTable* new, uint64_t sz);
void freewalk(PhysPageNum ppn);
void proc_free_page_table(PageTable* page_table, uint64_t sz);

#endif //MY_RISCV_OS_ADDRESS_H
