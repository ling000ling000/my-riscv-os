//
// Created by kk on 2026/2/4.
//

#ifndef MY_RISCV_OS_TYPES_H
#define MY_RISCV_OS_TYPES_H

// 定义无符号整型
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// RISCV64的寄存器大小是64位
typedef uint64_t reg_t;

#define EOF -1
#ifndef NULL
#define NULL ((void*)0)
#endif
#define EOS '\0'

#endif //MY_RISCV_OS_TYPES_H