#include "../include/os.h"

// 计算字符串长度
size_t strlen(const char* str)
{
    char *ptr = (char*)str;
    while (*ptr != EOS)
    {
        ptr ++;
    }
    return ptr - str;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    char* d = dest;
    while (n -- )
    {
        *d ++ = *((char*)(src ++));
    }
    return dest;
}

//复制字符 ch（一个无符号字符）到参数 dest 所指向的字符串的前 n 个字符。
void* memset(void *dest, int ch, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}