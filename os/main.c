#include "os.h"

void os_main()
{
    printf("str=%s char=%c percent=%%\n", "OK", 'A');
    panic(0);
}