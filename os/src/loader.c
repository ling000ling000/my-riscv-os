#include "../include/loader.h"

extern uint64_t _num_app[];
extern PageTable kernel_pagetable;
extern char _app_names[];
static char* app_names[MAX_TASKS];

#define USER_APP_STRIDE 0x20000UL

size_t get_num_app()
{
    return _num_app[0]; // _num_app[0] 存储的是应用总数
}

AppMetaData get_app_data(size_t app_id)
{
    AppMetaData meta_data;
    size_t num_app = get_num_app();

    meta_data.start = _num_app[app_id]; // 从_num_app数组中读取第app_id个应用的起始地址
    meta_data.size = _num_app[app_id + 1] - _num_app[app_id]; // 计算应用的大小：下一个地址 - 起始地址

    assert(app_id <= num_app);
    return meta_data;
}

void get_app_name()
{
    int app_num = get_num_app();
    printk("****----APPs----****\n");

    char *current_pos = (char *)_app_names;
    for (size_t i = 0; i < app_num; i++ )
    {
        app_names[i] = current_pos; // 记录当前字符串起始地址
        printk("%s\n", app_names[i]);
        current_pos += strlen(current_pos) + 1;
    }
    printk("********************\n");
}

size_t get_app_num_by_name(const char* app_name)
{
    size_t app_id = -1;
    int app_num = get_num_app();
    for (size_t i = 0; i < app_num; i++ )
    {
        if (strcmp(app_name, app_names[i]) == 0)
        {
            app_id = i;
            return app_id;
        }
    }
    return app_id; // 错误的情况
}

AppMetaData get_app_data_by_name(const char* path)
{
    AppMetaData meta_data = {0};
    int app_num = get_num_app();
    for (size_t i = 0; i < app_num; i ++ )
    {
        if (strcmp(path, app_names[i]) == 0)
        {
            // _num_app[0] stores app count, app payload starts from index 1.
            meta_data = get_app_data(i + 1);
            printk("[loader]find app: %s\n", path);
            return meta_data;
        }
    }
    panic("[loader]app not found: %s\n", path);
    return meta_data;
}

// 将ELF段权限标志（PF_R/PF_W/PF_X）转换为页表项（PTE）权限
static uint8_t flags_to_mmap_prot(uint8_t flags)
{
    // 按位转换：ELF的PF_* 映射到页表的PTE_*
    return  (flags & PF_R ? PTE_R : 0) | // 可读 → PTE_R
            (flags & PF_W ? PTE_W : 0) | // 可写 → PTE_W
            (flags & PF_X ? PTE_X : 0);  // 可执行 → PTE_X
}

void elf_check(elf64_ehdr_t* ehdr)
{
    assert(*(uint32_t *)ehdr == ELFMAG); // 判断elf文件的魔数
    // 验证架构和位数
    if (ehdr->e_machine != EM_RISCV || ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        panic("only riscv64 elf file is supported");
    }
}

void load_segment(size_t app_id, elf64_ehdr_t* ehdr, struct TaskControlBlock* proc)
{
    uint64_t app_base_off = app_id * USER_APP_STRIDE;
    AppMetaData meta_data = get_app_data(app_id + 1); // 获取应用程序的元数据（起始地址、大小）

    uint64_t entry = (uint64_t)ehdr->e_entry + app_base_off;
    proc->entry = entry;
    elf64_phdr_t* phdr;
    for (size_t i = 0; i < ehdr->e_phnum; i ++ )
    {
        phdr = (elf64_phdr_t *)(ehdr->e_phoff + ehdr->e_phentsize * i + meta_data.start); // 计算第i个程序头地址
        if (phdr->p_type == PT_LOAD)
        {
            uint64_t start_va = phdr->p_vaddr + app_base_off; // 映射内存段开始位置
            proc->ustack = start_va + phdr->p_memsz; // 映射内存段结束位置
            uint8_t map_perm = flags_to_mmap_prot(phdr->p_flags) | PTE_U; // 转换ELF程序段的权限标志为页表项权限
            uint64_t map_size = PGROUNDUP(phdr->p_memsz); // 计算该段需要映射的内存大小，向上对齐到页大小

            for (size_t j = 0; j < map_size; j += PAGE_SIZE)
            {
                PhysPageNum ppn = kalloc();
                uint64_t paddr = phys_addr_from_phys_page_num(ppn).value;
                // 仅拷贝文件中存在的数据，BSS 部分保持 kalloc 后的零值。
                uint64_t copy_size = 0;
                if (j < phdr->p_filesz)
                {
                    copy_size = phdr->p_filesz - j;
                    if (copy_size > PAGE_SIZE) copy_size = PAGE_SIZE;
                    memcpy((void *)paddr, (void *)(meta_data.start + phdr->p_offset + j), copy_size);
                }
                printk("proc->pagetable.value:%p\n", proc->page_table.root_ppn.value);
                // 建立虚拟地址到物理地址的映射
                PageTable_map(&proc->page_table,
                           virt_addr_from_size_t(start_va + j),
                          phys_addr_from_size_t(paddr),
                         PAGE_SIZE,
                              map_perm);
                // 同时映射到内核页表（兼容模式：不切 satp 也可运行）。
                // exec() 重载已有应用时，这个 VA 可能已在内核页表中存在，避免重复映射触发断言。
                VirtPageNum kva_vpn = floor_virts(virt_addr_from_size_t(start_va + j));
                PageTableEntry *kva_pte = find_pte(&kernel_pagetable, kva_vpn);
                if (!(kva_pte && PageTableEntry_is_valid(kva_pte)))
                {
                    PageTable_map(&kernel_pagetable,
                               virt_addr_from_size_t(start_va + j),
                              phys_addr_from_size_t(paddr),
                             PAGE_SIZE,
                                  map_perm);
                }
            }
        }
    }
    // 调整用户栈的最终位置：
    // 1. 先将临时栈地址向上对齐到页大小
    // 2. 再偏移2个页大小，预留足够的栈空间（避免和程序段重叠）
    proc->ustack = 2 * PAGE_SIZE + PGROUNDUP(proc->ustack);
    proc->base_size = proc->ustack; // 虚拟地址空间的最大值
}

void proc_ustack(struct TaskControlBlock* proc)
{
    // 分配一个物理页作为用户栈的内存空间
    PhysPageNum ppn = kalloc();
    uint64_t paddr = phys_addr_from_phys_page_num(ppn).value;

    // 映射用户栈的虚拟地址到物理地址：
    // 虚拟地址：proc->ustack - PAGE_SIZE（栈从高地址向低地址增长）
    // 权限：PTE_R(可读) | PTE_W(可写) | PTE_U(用户态)（栈不需要执行权限）
    // 大小：1个页（PAGE_SIZE）
    PageTable_map(&proc->page_table, virt_addr_from_size_t(proc->ustack - PAGE_SIZE),
                  phys_addr_from_size_t(paddr), PAGE_SIZE, PTE_R | PTE_W | PTE_U);
    VirtPageNum kva_vpn = floor_virts(virt_addr_from_size_t(proc->ustack - PAGE_SIZE));
    PageTableEntry *kva_pte = find_pte(&kernel_pagetable, kva_vpn);
    if (!(kva_pte && PageTableEntry_is_valid(kva_pte)))
    {
        PageTable_map(&kernel_pagetable, virt_addr_from_size_t(proc->ustack - PAGE_SIZE),
                      phys_addr_from_size_t(paddr), PAGE_SIZE, PTE_R | PTE_W | PTE_U);
    }
}

// 加载指定ID的应用程序到内存，并创建对应的进程控制块
void load_app(size_t app_id)
{
    uint64_t app_base_off = app_id * USER_APP_STRIDE;
    AppMetaData meta_data = get_app_data(app_id + 1); // 获取应用程序的元数据（起始地址、大小）
    elf64_ehdr_t* ehdr = (elf64_ehdr_t*)(meta_data.start); // 将应用起始地址转为ELF头部指针

    elf_check(ehdr);
    // assert(*(uint32_t *)ehdr == ELFMAG); // 判断elf文件的魔数
    // // 验证架构和位数
    // if (ehdr->e_machine != EM_RISCV || ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    // {
    //     panic("only riscv64 elf file is supported");
    // }

    // uint64_t entry = (uint64_t)ehdr->e_entry + app_base_off;
    TaskControlBlock* proc = task_create_pt(app_id);
    load_segment(app_id, ehdr, proc);
    // elf64_phdr_t* phdr;
    // for (size_t i = 0; i < ehdr->e_phnum; i ++ )
    // {
    //     phdr = (elf64_phdr_t *)(ehdr->e_phoff + ehdr->e_phentsize * i + meta_data.start); // 计算第i个程序头地址
    //     if (phdr->p_type == PT_LOAD)
    //     {
    //         uint64_t start_va = phdr->p_vaddr + app_base_off; // 映射内存段开始位置
    //         proc->ustack = start_va + phdr->p_memsz; // 映射内存段结束位置
    //         uint8_t map_perm = flags_to_mmap_prot(phdr->p_flags) | PTE_U; // 转换ELF程序段的权限标志为页表项权限
    //         uint64_t map_size = PGROUNDUP(phdr->p_memsz); // 计算该段需要映射的内存大小，向上对齐到页大小
    //
    //         for (size_t j = 0; j < map_size; j += PAGE_SIZE)
    //         {
    //             PhysPageNum ppn = kalloc();
    //             uint64_t paddr = phys_addr_from_phys_page_num(ppn).value;
    //             // 仅拷贝文件中存在的数据，BSS 部分保持 kalloc 后的零值。
    //             uint64_t copy_size = 0;
    //             if (j < phdr->p_filesz)
    //             {
    //                 copy_size = phdr->p_filesz - j;
    //                 if (copy_size > PAGE_SIZE) copy_size = PAGE_SIZE;
    //                 memcpy((void *)paddr, (void *)(meta_data.start + phdr->p_offset + j), copy_size);
    //             }
    //             printk("proc->pagetable.value:%p\n", proc->page_table.root_ppn.value);
    //             // 建立虚拟地址到物理地址的映射
    //             PageTable_map(&proc->page_table,
    //                        virt_addr_from_size_t(start_va + j),
    //                       phys_addr_from_size_t(paddr),
    //                      PAGE_SIZE,
    //                           map_perm);
    //             // 同时映射到内核页表（兼容模式：不切 satp 也可运行）。
    //             PageTable_map(&kernel_pagetable,
    //                        virt_addr_from_size_t(start_va + j),
    //                       phys_addr_from_size_t(paddr),
    //                      PAGE_SIZE,
    //                           map_perm);
    //         }
    //     }
    // }
    // // 调整用户栈的最终位置：
    // // 1. 先将临时栈地址向上对齐到页大小
    // // 2. 再偏移2个页大小，预留足够的栈空间（避免和程序段重叠）
    // proc->ustack = 2 * PAGE_SIZE + PGROUNDUP(proc->ustack);

    proc_ustack(proc);
    // 分配一个物理页作为用户栈的内存空间
    // PhysPageNum ppn = kalloc();
    // uint64_t paddr = phys_addr_from_phys_page_num(ppn).value;
    //
    // // 映射用户栈的虚拟地址到物理地址：
    // // 虚拟地址：proc->ustack - PAGE_SIZE（栈从高地址向低地址增长）
    // // 权限：PTE_R(可读) | PTE_W(可写) | PTE_U(用户态)（栈不需要执行权限）
    // // 大小：1个页（PAGE_SIZE）
    // PageTable_map(&proc->page_table, virt_addr_from_size_t(proc->ustack - PAGE_SIZE),
    //               phys_addr_from_size_t(paddr), PAGE_SIZE, PTE_R | PTE_W | PTE_U);
    // PageTable_map(&kernel_pagetable, virt_addr_from_size_t(proc->ustack - PAGE_SIZE),
    //               phys_addr_from_size_t(paddr), PAGE_SIZE, PTE_R | PTE_W | PTE_U);
    //
    // proc->base_size = proc->ustack; // 虚拟地址空间的最大值
}
