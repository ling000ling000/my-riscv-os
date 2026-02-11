#include "../include/stdio.h"

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
int _vsnprintf(char *out, size_t n, const char *s, va_list vl)
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