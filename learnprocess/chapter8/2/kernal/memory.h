#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
//第一次
/* 虚拟地址池，用于虚拟地址管理 */
struct virtual_addr{
    struct bitmap vaddr_bitmap;   // 虚拟地址用到了位图结构
    uint32_t vaddr_start;   // 虚拟地址起始地址
};

extern struct pool kernel_pool,user_pool;
void mem_init(void);
#endif
