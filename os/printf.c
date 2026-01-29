#include "os.h"

void uart_puts(const char *s)
{
    while (*s) {
        sbi_console_putchar(*s++);
    }
}

/*
 * 类似 libc 中的 vsnprintf
 *
 * out == NULL ：只计算最终字符串长度
 * out != NULL ：将格式化结果写入 out
 *
 * n ：输出缓冲区大小
 * s ：格式字符串
 * vl：可变参数列表
 *
 * 返回值：理论上应该输出的字符数（不含 '\0'）
 */
static int _vsnprintf(char *out, size_t n, const char *s, va_list vl)
{
    (void)vl;
    size_t pos = 0;
    for (; *s; s++)
    {
        if (out && pos < n)
        {
            out[pos] = *s;
        }
        pos ++;
    }
    if (out && pos < n)
    {
        out[pos] = 0;
    }
    else if (out && n)
    {
        out[n - 1] = 0;
    }
    return (int)pos;
}

/*
 * vprintf 实现
 *
 * 流程：
 * 1. 先调用 _vsnprintf(NULL) 计算输出长度
 * 2. 再真正格式化字符串
 * 3. 通过 UART 输出
 */
static char out_buf[1000]; // printf的全局输出缓冲区,所有printf最终都会先写入这里
static int _vprintf(const char *s, va_list vl)
{
    // 第一次调用：只计算长度
    int res = _vsnprintf(NULL, (size_t)-1, s, vl);

    // 防止缓冲区溢出
    if (res + 1 >= sizeof(out_buf))
    {
        uart_puts("error: output string size overflow\n");
        while (1) {}
    }

    // 第二次调用：真正生成字符串
    _vsnprintf(out_buf, res + 1, s, vl);

    // 输出到串口
    uart_puts(out_buf);

    return res;
}

static char msg_panic[] = "PANIC\n";

int printf(const char* fmt, ...)
{
    int res = 0;
    va_list vl;

    // 初始化可变参数
    va_start(vl, fmt);

    res = _vprintf(fmt, vl);

    va_end(vl);
    return res;
}

void panic(const char *fmt, ...)
{
    (void)fmt;
    uart_puts(msg_panic);
    while (1) {}
}