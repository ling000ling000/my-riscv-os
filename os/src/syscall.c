#include "../include/os.h"
#include "../include/address.h"
#include "../include/assert.h"

void __sys_yield(void);
void __sys_exit(int code);   // 如果你也会调用它

char* translated_byte_buffer(const char* data , size_t len)
{
    uint64_t user_satp = current_user_token();
    PageTable  pt ;
    pt.root_ppn.value = MAKE_PAGETABLE(user_satp);

    uint64_t start_va = (uint64_t)data;
    VirtPageNum vpn = floor_virts(virt_addr_from_size_t(start_va));
    PageTableEntry* pte = find_pte(&pt, vpn);
    if (pte == NULL || !PageTableEntry_is_valid(pte))
    {
        panic("[syscall]translated_byte_buffer: invalid user address");
    }

    // PTE[53:10] 是物理页号，转回物理地址
    uint64_t phyaddr = (pte->bits >> 10) << PAGE_SIZE_BITS;
    uint64_t page_offset = start_va & 0xFFF;
    const char *src = (const char *)(phyaddr + page_offset);
    return (char *)src;
}


// 定义写操作的具体实现函数
// 参数分别映射为：文件描述符 (fd)、数据缓冲区指针 (data)、数据长度 (len)
uint64_t __sys_write(size_t fd, const char* data, size_t len)
{
    // 在标准 POSIX 中，1 代表标准输出 stdout
    if (fd == stdin || fd == stdout)
    {
        char* str = translated_byte_buffer(data,len);
        printk("%s", str);
    }
    else
    {
        // 报告错误并终止运行：当前实现仅支持写入标准输出，不支持其他文件描述符
        panic("[syscall]Unsupported fd in sys_write!");
    }
    return 0;
}

void __sys_read(size_t fd, char* data, size_t len)
{
    if (fd == stdin)
    {
        int c;
        assert(len == 1);
        while (1)
        {
            c = sbi_console_getchar();
            if (c != -1)
                break;
            schedule();
            continue;
        }
        char* str = translated_byte_buffer(data, len);
        str[0] = c;
    }
}

void __sys_exit(int code)
{
    printk("[kernel] task exit code=%d\n", code);
    // 你可以把当前任务标记为 Exited，然后 schedule()
    // TODO:task_exit_current(); // 你自己实现，或者直接改 tasks[_current].task_state = Exited;
    schedule();
    panic("[syscall]unreachable in __sys_exit");
}

uint64_t __sys_get_time()
{
    return get_time_us();
}

uint64_t __sys_exec(const char* name)
{
    // Do not dereference user pointer directly in S-mode (e.g. strlen(name)),
    // otherwise it may fault on U pages when SUM is not enabled.
    char* app_name = translated_byte_buffer(name, 0);
    printk("[syscall]exec app name=%s\n", app_name);
    ex
    ec(app_name);
    return 0;
}


uint64_t __SYSCALL(size_t syscall_id, reg_t arg1, reg_t arg2, reg_t arg3)
{
    // 兼容路径：在某些运行路径下 ecall 触发点会早于用户态封装把 syscall id 移入 a7，
    // 此时 a7=0，而 a0 仍然是原始 syscall id。
    if (syscall_id == 0 && (arg1 == __NR_write || arg1 == __NR_shced_yield || arg1 == __NR_gettimeofday))
    {
        syscall_id = arg1;
        arg1 = arg2;
        arg2 = arg3;
        arg3 = 0;
    }

    // 根据传入的系统调用号进行分支选择
    switch (syscall_id)
    {
    case __NR_write:
        {
            return __sys_write(arg1, (const char *)arg2, arg3);
        }
    case __NR_shced_yield:
        {
            __sys_yield();
            break;
        }
    case __NR_gettimeofday:
        {
            uint64_t time = __sys_get_time();
            return time;
        }
    case __NR_read:
        {
            __sys_read(arg1, (char *)arg2, arg3);
            return 0;
        }
    case __NR_clone:
        {
            return __sys_fork();
        }
    case __NR_execve:
        {
            return __sys_exec(arg2);
        }
    default:
        {
            panic("[syscall]unsupport syscall id:%d\n", syscall_id);
            break;
        }
    }
    return 0;
}

void __sys_yield()
{
    schedule();
}
