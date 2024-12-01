#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
//第一次
// /* 虚拟地址池，用于虚拟地址管理 */
// struct virtual_addr{
//     struct bitmap vaddr_bitmap;   // 虚拟地址用到了位图结构
//     uint32_t vaddr_start;   // 虚拟地址起始地址
// };

// extern struct pool kernel_pool,user_pool;
// void mem_init(void);
// #endif
//改了之后
/* 内存池标记，用于判断用哪个内存池 */
enum pool_flags{
    PF_KERNEL=1,   // 内核内存池
    PF_USER=2   // 用户内存池
};

#define PG_P_1  1   // 存在位
#define PG_P_0  0
#define PG_RW_R 0   // R/W位，读/执行
#define PG_RW_W 2   //        读/写/执行
#define PG_US_S 0   // U/S位，系统级
#define PG_US_U 4   //        用户级

struct virtual_addr{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

extern struct pool kernel_pool,user_pool;
void mem_init(void);
void* vaddr_get(enum pool_flags pf,uint32_t pg_cnt);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
void* palloc(struct pool* m_pool);
void page_table_add(void* _vaddr,void* _page_phyaddr);
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt);
void* get_kernel_pages(uint32_t pg_cnt);
void mem_pool_init(uint32_t all_mem);
#endif