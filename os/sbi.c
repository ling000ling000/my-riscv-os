#include "sbi.h"
#include "stdint.h"
struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
			unsigned long arg1, unsigned long arg2,
			unsigned long arg3, unsigned long arg4,
			unsigned long arg5)
{
	struct sbiret ret;

    //使用GCC的扩展语法，用于将一个值存储到RISC-V架构中的寄存器a0中。
	register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0);
	register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);
	register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);
	register uintptr_t a3 asm ("a3") = (uintptr_t)(arg3);
	register uintptr_t a4 asm ("a4") = (uintptr_t)(arg4);
	register uintptr_t a5 asm ("a5") = (uintptr_t)(arg5);
	register uintptr_t a6 asm ("a6") = (uintptr_t)(fid);
	register uintptr_t a7 asm ("a7") = (uintptr_t)(ext);
	asm volatile ("ecall"
		      : "+r" (a0), "+r" (a1)
		      : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
		      : "memory");
	ret.error = a0;
	ret.value = a1;

	return ret;
}


/**
 * sbi_console_putchar() - Writes given character to the console device.
 * @ch: The data to be written to the console.
 *
 * Return: None
 */
void sbi_console_putchar(int ch)
{
	sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0, ch, 0, 0, 0, 0, 0);
}

/**
 * sbi_set_timer() - 设定下一次定时器事件（中断）的触发时刻
 * @stime_value: 下一次定时器中断应当触发的绝对时间值 (absolute time)。
 * 当系统的 mtime 寄存器值达到该值时，产生时钟中断。
 *
 * Return: None (无返回值)
 */
void sbi_set_timer(uint64_t stime_value)
{
    // 调用sbi接口
    sbi_ecall(SBI_EXT_TIME, // 参数 0 (Extension ID): 指定扩展 ID 为 "Time Extension" (时间扩展模块)
              SBI_FID_SET_TIMER, // 参数 1 (Function ID): 指定功能 ID 为 "Set Timer" (设置定时器)
              stime_value, // 参数 2: 传递给 SBI 的具体参数，即设定的目标时间值
              0, 0, 0, 0, 0 // 参数 3-7: 后续参数未被该功能使用，填充为 0 以满足函数签名
    );
}
