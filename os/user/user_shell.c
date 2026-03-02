#include "../include/types.h"
#include "../include/os.h"

#define LF 0x0a // 换行
#define CR 0x0d // 回车
#define DL 0x7f // 删除键
#define BS 0x08 // 退格键
#define BUFFER_SIZE 1024

int main()
{
    printf("my riscv os user shell\n");
    printf(">> ");
    char line[BUFFER_SIZE];
    size_t len = 0;
    line[0] = '\0';
    while (1)
    {
        char c = getchar(); // 阻塞等待用户输入字符
        switch (c)
        {
        // --- 情况 1: 用户按下回车键 ---
        case CR:
        case LF:
            {
                printf("\n");
                if (len > 0)
                {
                    int pid = sys_fork(); // 加载新程序
                    if (pid == 0)
                    {
                        sys_exec(line);
                    }
                }
                len = 0;
                line[0] = '\0';
                printf(">> ");
                break;
            }
        // --- 情况 2: 用户按下退格/删除键 ---
        case BS:
        case DL:
            {
                if (len > 0)
                {
                    printf("\b \b"); // 回显处理：光标左移一格，打印空格覆盖字符，再左移一格
                    len--;
                    line[len] = '\0'; // 更新缓冲区
                }
                break;
            }
        // --- 情况 3: 普通字符输入 ---
        default:
            {
                if (len < BUFFER_SIZE - 1)
                {
                    printf("%c", c);
                    line[len++] = c;
                    line[len] = '\0';
                }
                break;
            }
        }
    }
    return 0;
}
