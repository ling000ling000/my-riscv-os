#ifndef MY_RISCV_OS_LOADER_H
#define MY_RISCV_OS_LOADER_H

#include "types.h"
#include "assert.h"
#include "stdio.h"

#define EI_NIDENT 16 // ELF 头部e_ident数组的长度
#define ELFMAG 0x464C457FU // ELF魔数（0x7F 'E' 'L' 'F'），用于识别ELF文件

#define EM_RISCV 0xF3 // RISC-V架构对应的e_machine字段值（ELF标识运行架构）

#define EI_CLASS 4 // e_ident数组中标识ELF位数的下标（第5个字节，从0开始）
#define RLFCLASSNONE 0 // 无效的ELF位数
#define ELFCLASS32 1 // 32位ELF文件
#define ELFCLASS64 2 // 64位ELF文件（RISC-V内核通常用64位）
#define ELFCLASSNUM 3 // ELF位数类型总数

#define PT_LOAD 1 // 可加载段

#define PF_X 0x1 // 段可执行（代码段）
#define PF_W 0x2 // 段可写（数据段/栈）
#define PF_R 0x4 // 段可读（代码/数据段）

// ELF64文件头部结构体, 对应ELF文件的前64字节（64位架构），是解析ELF文件的入口
typedef struct {
    uint8_t e_ident[EI_NIDENT];    // ELF标识数组（16字节）：包含魔数、位数、字节序等
    uint16_t e_type;               // ELF文件类型（如ET_EXEC可执行文件、ET_REL重定位文件）
    uint16_t e_machine;            // 目标架构（如EM_RISCV=0xF3）
    uint32_t e_version;            // ELF版本（通常为EV_CURRENT=1）
    uint64_t e_entry;              // 程序入口地址（应用的main函数/启动函数地址）
    uint64_t e_phoff;              // 程序头表（Program Header）的偏移量（字节）
    uint64_t e_shoff;              // 节头表（Section Header）的偏移量（字节）
    uint32_t e_flags;              // 架构相关标志（RISC-V无特殊标志，通常为0）
    uint16_t e_ehsize;             // ELF头部自身的大小（字节）
    uint16_t e_phentsize;          // 每个程序头表项的大小（字节）
    uint16_t e_phnum;              // 程序头表项的数量（可加载段的数量）
    uint16_t e_shentsize;          // 每个节头表项的大小（字节）
    uint16_t e_shnum;              // 节头表项的数量
    uint16_t e_shstrndx;           // 节名字符串表在节头表中的索引
} elf64_ehdr_t;

// ELF64 程序头表结构体, 每个程序头表项对应一个可加载段（如代码段.text、数据段.data）
typedef struct {
    uint32_t p_type;               // 段类型（如PT_LOAD=1表示可加载段）
    uint32_t p_flags;              // 段权限（PF_R/PF_W/PF_X的组合）
    uint64_t p_offset;             // 段在ELF文件中的偏移量（字节）
    uint64_t p_vaddr;              // 段加载到内存后的虚拟地址
    uint64_t p_paddr;              // 段加载到内存后的物理地址（嵌入式/内核中常用）
    uint64_t p_filesz;             // 段在ELF文件中的大小（字节，如代码段的实际大小）
    uint64_t p_memsz;              // 段加载到内存后的大小（字节，可能大于p_filesz，如.bss段）
    uint64_t p_align;              // 段的对齐要求（通常为PAGE_SIZE=4KB）
} elf64_phdr_t;

typedef struct
{
    uint64_t start;
    uint64_t size;
} AppMetaData;

size_t get_num_app();
AppMetaData get_app_data(size_t app_id);
void get_app_name();
void load_app(size_t app_id);
AppMetaData get_app_data_by_name(const char* path);
size_t get_app_num_by_name(const char* app_name);
void elf_check(elf64_ehdr_t* ehdr);
void load_segment(size_t app_id, elf64_ehdr_t* ehdr, struct TaskControlBlock* proc);
void proc_ustack(struct TaskControlBlock* proc);

#endif //MY_RISCV_OS_LOADER_H
