#include "../include/loader.h"

extern uint64_t _num_app[];

size_t get_num_app()
{
    return _num_app[0]; // _num_app[0] 存储的是应用总数
}

AppMetaData get_app_data(size_t app_id)
{
    AppMetaData meta_data;
    size_t num_app = get_num_app();

    meta_data.start = _num_app[app_id]; // 从_num_app数组中读取第app_id个应用的起始地址
    meta_data.size = _num_app[app_id + 1] - _num_app[app_id]; // 计算应用的大小：下一个地址 - 起始地址

    printk("app start:%x , app size: %x\n", meta_data.start, meta_data.size);
    assert(app_id <= num_app);
    return meta_data;
}