//
// Created by kk on 2026/2/6.
//
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