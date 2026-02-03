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
    size_t pos = 0; // 初始化计数器，用于记录理论上需要写入的字符总数
    int format = 0; // 初始化状态标记：0 代表普通文本模式，1 代表正在解析格式符
    int longarg = 0; // 当遇到 %l 时只设置 longarg = 1，不输出，继续等待下一个格式字符

    for (; *s; s++) // 循环遍历格式化字符串 s，直到遇到结束符 \0
    {
        if (!format) // 如果当前处于普通文本模式
        {
            if (*s == '%')
            {
                format = 1;
                continue;
            }
            if (out && pos < n) out[pos] = *s; // 如果缓冲区有效且当前位置未超过缓冲区大小，将字符写入缓冲区
            pos++;
            continue;
        }

        // format == 1: 处理%后面的格式符
        switch (*s)
        {
        case 'l':
            {
                longarg = 1; // 设置长整型标志位，表示后续参数应被视为 long 类型
                continue;
            }
        case 's': // 处理字符串 (%s)
            {
                const char *s2 = va_arg(vl, const char *);
                if (!s2) s2 = "(null)";
                while (*s2)
                {
                    if (out && pos < n) out[pos] = *s2;
                    pos ++;
                    s2 ++;
                }
                break;
            }
        case 'c': // 处理单个字符 (%c)
            {
                char ch = (char)va_arg(vl, int);
                if (out && pos < n) out[pos] = ch;
                pos ++;
                break;
            }
        case 'x': // 处理十六进制整数 (%x)
            {
                // 根据 longarg 标志判断是取 unsigned long 还是 unsigned int
                unsigned long num = longarg ? va_arg(vl, unsigned long) : (unsigned long)va_arg(vl, unsigned int);
                // 计算需要输出的十六进制位数：字节数 * 2 (1字节 = 2个16进制位)
                int hexdigits = 2 * (longarg ? (int)sizeof(unsigned long) : (int)sizeof(unsigned int));

                // 从最高位到最低遍历
                for (int i = hexdigits - 1; i >= 0; i-- )
                {
                    // 右移 4*i 位将目标 4 位移到最低端，再用 & 0xF 取出这 4 位（即 0-15 的值）
                    int d = (num >> (4*i)) & 0xf;
                    // 将数值转换成字符：0-9 转 '0'-'9'，10-15 转 'a'-'f'
                    char ch = (d < 10) ? ('0' + d) : ('a' + d - 10);
                    if (out && pos < n) out[pos] = ch;
                    pos ++;
                }
                longarg = 0;
                break;
            }
        case 'p': // 处理指针地址 (%p)
            {
                unsigned long num = (unsigned long)va_arg(vl, void*);

                // 手搓0x前缀
                if (out && pos < n) out[pos] = '0';
                pos ++;
                if (out && pos < n) out[pos] = 'x';
                pos ++;

                // 指针的宽度固定取决于系统指针大小 (sizeof(void*))
                int hexdigits = 2 * (int)sizeof(void*);
                // 逐个半字节转换为字符
                for (int i = hexdigits - 1; i >= 0; i-- )
                {
                    int d = (num >> (4*i)) & 0xf;
                    char ch = (d < 10) ? ('0' + d) : ('a' + d - 10); // 提取当前 4 位
                    if (out && pos < n) out[pos] = ch;
                    pos ++;
                }
                longarg = 0;
                break;
            }
        case 'u': // 处理无符号十进制数
            {
                unsigned long num = longarg ? va_arg(vl, unsigned long) : (unsigned long)va_arg(vl, unsigned int);

                // 预计算该数字在十进制显示下的位数
                unsigned long tmp = num;
                int digits = 1;
                while (tmp >= 10)
                {
                    tmp /= 10;
                    digits ++;
                }

                // 将数值转换为字符并填充到缓冲区
                for (int i = digits - 1; i >= 0; i -- )
                {
                    char ch = '0' + (num % 10); // 取出当前数值的个位数字，并转换为 ASCII 字符

                    // 边界检查：必须检查 具体写入位置 (pos + i) 是否越界
                    if (out && pos + (size_t)i < n)
                    {
                        out[pos + (size_t)i] = ch;
                    }
                    num /= 10;
                }

                pos += (size_t)digits;

                longarg = 0;
                break;
            }
        case 'd': // 处理有符号十进制整数 (%d)
            {
                // 根据 longarg 标志，分别提取 long 或 int 类型，并统一转换为 long 类型方便后续处理
                long snum = longarg ? va_arg(vl, long) : (long)va_arg(vl, int);

                unsigned long num; // 用于存储该数的绝对值
                if (snum < 0)
                {
                    // 先处理符号位
                    if (out && pos < n) out[pos] = '-';
                    pos ++;

                    // 计算绝对值并转换为无符号数
                    num = (unsigned long)(-snum);
                }
                else
                {
                    // 如果是正数，直接转换为无符号数
                    num = (unsigned long)snum;
                }

                // 计算绝对值的位数
                unsigned long tmp = num;
                int digits = 1; // 默认至少 1 位
                while (tmp >= 10)
                {
                    tmp /= 10;
                    digits ++;
                }

                // 将数值转换为字符并填充到缓冲区
                for (int i = digits - 1; i >= 0; i -- )
                {
                    char ch = '0' + (num % 10); // 取出当前数值的个位数字，并转换为 ASCII 字符

                    // 边界检查：必须检查 具体写入位置 (pos + i) 是否越界
                    if (out && pos + (size_t)i < n)
                    {
                        out[pos + (size_t)i] = ch;
                    }
                    num /= 10;
                }

                pos += (size_t)digits;

                longarg = 0;
                break;
            }
        case '%':
            {
                if (out && pos < n) out[pos] = '%';
                pos ++;
                longarg = 0;
                break;
            }
        default:
            longarg = 0;
            break;
        }
        format = 0;
    }

    // 处理字符串的NULL结束符
    if (out && pos < n) out[pos] = 0; // 如果缓冲区空间充足，在内容末尾添加结束符
    else if (out && n) out[n - 1] = 0; // 如果缓冲区已满但长度不为0，在最后一位强制截断并添加结束符

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
    va_list vl2;
    va_copy(vl2, vl);

    // 第一次调用 _vsnprintf：
    // 1. 缓冲区传 NULL，表示不实际写入字符
    // 2. 长度传 (size_t)-1，利用无符号数溢出特性得到系统允许的最大整数 (SIZE_MAX)，防止长度受限
    // 3. 这一步的唯一目的是计算格式化后所需的字符串长度 res
    int res = _vsnprintf(NULL, (size_t)-1, s, vl);

    // 检查计算出的长度 res 加上 1 个结束符是否超过了全局缓冲区 out_buf 的容量
    if (res + 1 > (int)sizeof(out_buf))
    {
        uart_puts("error: output string size overflow\n");
        while (1) {}
    }

    // 第二次调用 _vsnprintf：
    // 1. 传入实际缓冲区 out_buf
    // 2. 传入确定的长度 res + 1 (包含结束符)
    // 3. 使用备份的变参列表 vl2，因为原 vl 在第一次调用中已经被消耗
    _vsnprintf(out_buf, res + 1, s, vl2);
    va_end(vl2); // 释放备份的变参列表 vl2，结束变参处理

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
    va_list vl;

    uart_puts("panic: ");

    va_start(vl, fmt);
    _vprintf(fmt, vl);
    va_end(vl);

    uart_puts("\n");
    while (1) {}
}