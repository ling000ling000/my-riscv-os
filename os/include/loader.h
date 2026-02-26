#ifndef MY_RISCV_OS_LOADER_H
#define MY_RISCV_OS_LOADER_H

#include "types.h"
#include "assert.h"
#include "stdio.h"

typedef struct
{
    uint64_t start;
    uint64_t size;
} AppMetaData;

size_t get_num_app();
AppMetaData get_app_data(size_t app_id);

#endif //MY_RISCV_OS_LOADER_H