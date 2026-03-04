#include "../include/types.h"
#include "../include/os.h"

int main()
{
    int step = 0;
    while (1)
    {
        printf("test_exit: %d\n", step);
        step ++;
        if (step > 50)
        {
            printf("exit!\n");
            sys_exit(50);
        }
    }
    return 0;
}