//
// Created by kk on 2026/2/9.
//
#include "os.h"

// 定义 QEMU virt 机器的硬件时钟频率
// 10MHz (10,000,000 Hz)，意味着 mtime 寄存器每秒增加 10,000,000 次
#define CLOCK_FREQ 10000000
// 定义操作系统每秒希望发生的时钟中断次数（节拍数/Ticks）
// 这里设为 100，意味着每秒触发 100 次中断，即时间片长度为 10ms
#define TICKS_PER_SEC 100

#define MICRO_PER_SEC 1000000UL

static uint64_t timebase = 1000000;

// 设置下次时间中断的触发时刻
void set_next_trigger()
{
    // 1. r_mtime(): 获取当前硬件时间（总周期数）
    // 2. CLOCK_FREQ / TICKS_PER_SEC: 计算每个时间片（Tick）对应的硬件周期数
    //    例如：10,000,000 / 100 = 100,000 周期
    // 3. 两者相加得到“下一次中断的绝对时刻”
    // 4. 调用 SBI 接口设置硬件比较器
    sbi_set_timer(r_mtime() + CLOCK_FREQ / TICKS_PER_SEC);
}

// 初始化并开启s mode的时钟中断
void timer_init()
{
    reg_t sie = r_sie();
    sie |= SIE_STIE; // SIE_STIE 通常定义为 (1 << 5)，置 1 表示允许处理时钟中断
    w_sie(sie);
    set_next_trigger();
}

// 获取当前时间
uint64_t get_time_us()
{
    // 计算公式：当前总周期数 / 每 Tick 周期数
    // CLOCK_FREQ / MICRO_PER_SEC = 100,000
    // r_mtime() / (CLOCK_FREQ / MICRO_PER_SEC) = 当前系统运行了多少个微秒
    reg_t time = r_mtime() / (CLOCK_FREQ / MICRO_PER_SEC);
    return time;
}