#include "os.h"

void os_main()
{
    printf("d=%d u=%u\n", -123, 123u);
    printf("ld=%ld lu=%lu\n", -123456789L, 123456789UL);
    printf("mix: %s %c %x %p %d\n", "OK", 'A', 0x1234abcd, (void*)0x80200000, -7);

    panic(0);
}