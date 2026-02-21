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