#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <dirent.h>


#define TARGET_PATH "./user/bin/"

// qsort排序的比较函数，用于对字符串数组按字典序排序
int compare_strings(const void *a, const void *b)
{
    return strcmp(*(const char**)a, *(const char**)b); // <0则a在前，>0则b在前，=0则相等
}

// 读取目录下的应用程序文件，生成汇编文件link_app.S
void insert_app_data()
{
    // 以写入模式打开（创建）src/link_app.S文件，用于生成汇编代码
    FILE* f = fopen("src/link_app.S", "w");
    if (f == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char* apps[100]; // 存储应用程序名称
    int app_count = 0;

    DIR* dir = opendir("./user/bin");
    if (dir == NULL)
    {
        perror("Failed to open dir");
        exit(EXIT_FAILURE);
    }

    struct dirent* dir_entry; // 存储每次读取到的目录项信息
    while ((dir_entry = readdir(dir)) != NULL)
    {
        const char* name_with_ext = dir_entry->d_name;
        if (name_with_ext[0] == '.' && (name_with_ext[1] == '\0' || (name_with_ext[1] == '.' && name_with_ext[2] == '\0')))
        {
            continue; // 排除目录中的.和..避免无效处理
        }

        int len = (int)strlen(name_with_ext);
        if (len <= 4 || strcmp(name_with_ext + len - 4, ".bin") != 0) {
            continue;
        }

        char* app_name = (char*)malloc((size_t)len - 3);
        if (app_name == NULL) {
            perror("Failed to alloc memory");
            exit(EXIT_FAILURE);
        }
        memcpy(app_name, name_with_ext, (size_t)len - 4);
        app_name[len - 4] = '\0';

        apps[app_count] = app_name; // 仅存储无扩展名的应用名
        app_count ++;
        printf("File name: %s, app_count: %d\n", app_name, app_count);
    }
    closedir(dir);

    qsort(apps, app_count, sizeof(char*), compare_strings); // 对应用名排序
    fprintf(f, "\n.align 3\n.section .data\n.global _num_app\n_num_app:\n.quad %d", app_count); // 写入汇编代码：定义应用数量的全局变量_num_app，值为app_count
    for (int i = 0; i < app_count; i ++ )
    {
        fprintf(f, "\n.quad app_%d_start", i); // 写入每个应用的起始地址索引
    }
    if (app_count > 0) {
        fprintf(f, "\n.quad app_%d_end", app_count - 1); // 写入最后一个应用的结束地址索引
    } else {
        fprintf(f, "\n.quad 0");
    }

    for (int i = 0; i < app_count; i ++ ) // 遍历每个应用，写入对应的汇编代码（嵌入二进制文件）
    {
        printf("app_%d: %s\n", i, apps[i]);
        // 写入汇编代码：
        // 1. 声明app_i_start和app_i_end为全局符号
        // 2. 按8字节对齐
        // 3. .incbin：将指定路径的二进制文件嵌入到汇编代码中
        // 4. app_i_end：标记该应用二进制数据的结束位置
        fprintf(f,
                "\n.section .data\n.global app_%d_start\n.global app_%d_end\n.align 3\napp_%d_start:\n.incbin \"%s%s.bin\"\napp_%d_end:",
                i, i, i,
                TARGET_PATH,
                apps[i], i);
        free(apps[i]); // 释放strdup分配的堆内存，避免内存泄漏
    }
    fclose(f);
}

int main()
{
    insert_app_data();
    return 0;
}
