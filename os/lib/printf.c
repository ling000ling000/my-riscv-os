#include "../include/os.h"
#include "../include/stdio.h"

static char out_buf[1000]; // printf的全局输出缓冲区,所有printf最终都会先写入这里
static int _vprintf(const char *s, va_list vl)
{
    int res = _vsnprintf(NULL, -1, s, vl); // 这一步仅仅是为了计算格式化后的字符串会有多长，返回值 res 是字符数
    _vsnprintf(out_buf, res + 1, s, vl); //
    sys_write(stdout, out_buf, res + 1); // 调用系统调用
    return res;
}

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
