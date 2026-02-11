#include "../include/os.h"

void os_main()
{
    printk("hello world!\n");

    trap_init();

    task_init();

    timer_init();  // 允许时钟中断

    set_next_trigger();        // 启动第一次中断

    printk("[os] before run_first_task\n");
    run_first_task();
    printk("[os] after run_first_task (should never happen)\n");
}