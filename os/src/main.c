#include "loader.h"
#include "../include/os.h"
#include "../include/assert.h"

void os_main()
{
    printk("hello world!\n");
    frame_alloctor_init(); // 内存分配器初始化
    size_t app_num = get_num_app();
    printk("app num: %d\n", app_num);
    kvminit();

    for (size_t i = 0; i < app_num; i++)
    {
        load_app(i);
        app_init(i);
    }
    asm volatile("fence.i");

    kvminithart(); // 映射内核
    trap_init(); // 设置用户陷入入口为 __alltraps
    timer_init();
    run_first_task();
    panic("run_first_task returned unexpectedly");
}
