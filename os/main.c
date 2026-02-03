#include "os.h"

void os_main()
{
    int v = 0x1234abcd;
    void *p = (void *)&v;

    printf("hex=%x\n", v);
    printf("ptr=%p\n", p);
    printf("longhex=%lx\n", (unsigned long)0x1122334455667788UL);

    panic(0);
}