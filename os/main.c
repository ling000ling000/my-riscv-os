#include "os.h"

void os_main()
{
    printf("hello world!\n");

    trap_init();

    task_init();

    timer_init();  // 允许时钟中断

    set_next_trigger();        // 启动第一次中断

    printf("[os] before run_first_task\n");
    run_first_task();
    printf("[os] after run_first_task (should never happen)\n");
}