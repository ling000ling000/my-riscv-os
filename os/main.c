#include "os.h"

void os_main()
{
    printf("hello world!\n");

    trap_init();

    task_init();

    printf("[os] before run_first_task\n");
    run_first_task();
    printf("[os] after run_first_task (should never happen)\n");
}