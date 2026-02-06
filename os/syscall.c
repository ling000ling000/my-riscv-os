//
// Created by kk on 2026/2/6.
//
#include "os.h"

void __SYSCALL(size_t syscall_id, reg_t arg1, reg_t arg2, reg_t arg3)
{
    // 根据传入的系统调用号进行分支选择
    switch (syscall_id)
    {
        case sys_write:
            {
                // TODO:__sys_write();
                break;
            }
        default:
            {
                panic("unsupport syscall id");
                break;
            }
    }
}

// 定义写操作的具体实现函数
// 参数分别映射为：文件描述符 (fd)、数据缓冲区指针 (data)、数据长度 (len)
void __sys_write(size_t fd, const char* data, size_t len)
{
    // 在标准 POSIX 中，1 代表标准输出 stdout
    if (fd == 1)
    {
        // 如果是标准输出，直接调用 printf 将数据打印到屏幕/串口
        // 注意：此处直接打印 data，未利用 len 参数，这在教学代码中常见但并不严谨
        printf(data);
    }
    else
    {
        // 报告错误并终止运行：当前实现仅支持写入标准输出，不支持其他文件描述符
        panic("Unsupported fd in sys_write!");
    }
}