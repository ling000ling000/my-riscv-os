//
// Created by kk on 2026/2/4.
//
#include <stddef.h>
#include "os.h"
#define USER_STACK_SIZE (4096 * 2)
#define KERNEL_STACK_SIZE (4096 * 2)

uint8_t kernel_stack[USER_STACK_SIZE];
uint8_t user_stack[KERNEL_STACK_SIZE];