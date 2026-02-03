#include "os.h"

void os_main()
{
    int x = -10;
    void *p = (void*)0x80200000;

    panic("bad x=%d ptr=%p hex=%x", x, p, 0xdeadbeef);
}