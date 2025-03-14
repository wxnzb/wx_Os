//改了之后；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；；
#include "memory.h"
#include "bitmap.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "print.h"
#include "string.h"

#define PG_SIZE 4096

/************** 位图地址 *****************/
#define MEM_BITMAP_BASE 0xc009a000

/* 0xc0000000是内核从虚拟地址3G起，0x100000意指跨过低端1MB内存，使虚拟地址在逻辑上连续 */
#define K_HEAP_START 0xc0100000

//0xffc00000为 1111 1111 1100 0000 0000 0000 0000 0000,这样&出来可以得到高10位
#define PDE_IDX(addr) ((addr & 0xffc00000)>>22)   // 得到PDX(页目录项)
//0x003ff000为 0000 0000 0011 1111 1111 0000 0000 0000,这样&出来可以得到中间10位
#define PTE_IDX(addr) ((addr & 0x003ff000)>>12)   // 得到PTX(页表项)
//我现在有个疑问：上面那个位图管理的是虚拟地址空间，下面那个管理的是物理地址空间，又分为内核和用户，所以一共有4个位图，他们存放在哪里？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
// struct virtual_addr{
//     struct bitmap vaddr_bitmap;
//     uint32_t vaddr_start;
// };
// 放在这好看
/* 内存池结构，生成两个实例用于管理内核内存池和用户内存池 */
struct pool{
    struct bitmap pool_bitmap;   // 本内存池用到的位图结构，用于管理物理内存
    uint32_t phy_addr_start;   // 本内存池所管理物理内存的起始地址
    uint32_t pool_size;   // 本内存池字节容量
};

struct pool kernel_pool,user_pool;   // 生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr;   // 此结构用来给内核分配虚拟地址

/* 在pf表示的虚拟内存池中申请pg_cnt个虚拟页
 * 成功则返回虚拟页的起始地址，失败则返回NULL */
//？？？？？？？？？？？？？？？？？？？这里也没有初始化kernel_vaddr这个结构体阿
void* vaddr_get(enum pool_flags pf,uint32_t pg_cnt){
    int vaddr_start=0,bit_idx_start=-1;
    uint32_t cnt=0;
    if (pf==PF_KERNEL){
    bit_idx_start=bitmap_scan(&kernel_vaddr.vaddr_bitmap,pg_cnt);
    if (bit_idx_start==-1){
        return NULL;
    }
//要是找到了pg_cnt个空闲虚拟页(他是通过位图为0去找)，将他们全部置为1
    while (cnt<pg_cnt){
        bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start+cnt++,1);
    }
//内核虚拟内存池的起始地址+上面说了kernel_vaddr.vaddr_bitmap证明这是内核的位图，好抽象没看懂？？？？？？？
    vaddr_start=kernel_vaddr.vaddr_start+bit_idx_start*PG_SIZE;
    }else{
    //用户内存池，将来实现用户进程再补充
    }
    return (void*)vaddr_start;
}
/* 得到虚拟地址vaddr对应的pte指针 */
uint32_t* pte_ptr(uint32_t vaddr){
    /* 先访问到页表自己 +
     * 再用页目录项pde作为pte的索引访问到页表 +
     * 再用页表项pte作为页内偏移 */
//0xffc00000是 页目录表的基地址，至于为什么是这个值，不清楚
//我现在感觉这样才正确？？(uint32_t*)(0xffc00000 +(((vaddr & 0xffc00000) >> 22) +PTE_IDX(vaddr)) * 4);    
//还是要看书，果然这里陷阱很多
//页表项里面有1024个页目录项，他里面存有指向页表的物理地址，通过高10位可以找到页目录项，页表里面有1024个页表项，通过中间10位可以找到页表项，页表项里面存有物理页的地址，通过低12位可以找到页内偏移
//这里我们一直在欺骗处理器
//pte他只需要高22位就行了，他的目的是找到相应的页表项
//首先0xffc00000是高10位全为1的情况，也就是指向最后一项页目录项，一般页目录项里面村的是页表的地址，但是这个最后一项村的是第一个页目录项的地址，也就是页目录表的起始地址 
//((vaddr & 0xffc00000)>>10)他虽然处于中间10位，但是他是页目录项的索引
//  PTE_IDX(vaddr)他是页表项的索引，因为要12位所以给他*4       
    uint32_t* pte=(uint32_t*)(0xffc00000+\
        ((vaddr & 0xffc00000)>>10)+\
        PTE_IDX(vaddr)*4);
    return pte;
}

/* 得到虚拟地址vaddr对应的pde指针 */
uint32_t* pde_ptr(uint32_t vaddr){
    /* 0xfffff用来访问到页表本身所在的地址 */
//页目录表的基地址 是 0xFFFF0000这到底是个啥？？？？？？？？
//(0xfffff000)高20位全为1,通过高10位得到页目录表物理地址，中间10位我在感觉没啥用？？？？，最后PDE_IDX(vaddr)是页目录项索引*4刚好12位
    uint32_t* pde=(uint32_t*)((0xfffff000)+PDE_IDX(vaddr)*4);
    return pde;
}

/* 在m_pool指向的物理内存池中分配1个物理页，
 * 成功则返回页框的物理地址，失败则返回NULL */
void* palloc(struct pool* m_pool){
    /* 扫描或设置位图要保证原子操作 */
    int bit_idx=bitmap_scan(&m_pool->pool_bitmap,1);   // 找一个物理页面
    if (bit_idx==-1){
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1);   // 将此位bit_idx置为1
    uint32_t page_phyaddr=((bit_idx*PG_SIZE)+m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}

/* 页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射 */
void page_table_add(void* _vaddr,void* _page_phyaddr){
    uint32_t vaddr=(uint32_t)_vaddr,page_phyaddr=(uint32_t)_page_phyaddr;
    uint32_t* pde=pde_ptr(vaddr);
    uint32_t* pte=pte_ptr(vaddr);

    /***************************** 注意 ****************************
     * 执行*pte，会访问到空的pde。所以确保pde创建完成后才嫩执行*pte,
     * 否则会引发page_fault。因此在*pde为0时，pte只能在下面else语句块的*pde后面。
     **************************************************************/
    /* 先在页目录内判断目录项的P位，若为1，则表示该表已存在 */
    if (*pde & 0x00000001){   //P位，此处判断目录项是否存在。若存在
    ASSERT(!(*pte & 0x00000001));

    if(!(*pte & 0x00000001)){
        *pte=(page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }else{   // 理论上不会执行到这里
        PANIC("pte repeat");
        *pte=(page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
    }else{   // 页目录项不存在，所以要先创建页目录项再创建页表项
        /* 页表中用到的页框一律从内核空间分配 */
       uint32_t pde_phyaddr=(uint32_t)palloc(&kernel_pool);
//这块下面不是很懂？？？？？？？？？？？？？
       *pde=(pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

       /* 分配到的物理页地址pde_phyaddr对应的物理内存清0，
    * 避免里面的陈旧数据变成了页表项，从而让页表混乱。
    * 访问到pde对应的物理地址，用pte取高20位即可。
    * 因为pte基于该pde对应的物理地址内再寻址，
    * 把低12位置0便是该pde对应的物理页的起始 */
//pte指向页表项，由上面值高20位指向的是页目录项，将这个页目录项里面的页表全部清0？？？？？？？
//不是，这时pte和新分配的pde还有啥关系吗？？？？？？？？？
       memset((void*)((int)pte & 0xfffff000),0,PG_SIZE);

       ASSERT(!(*pte & 0x00000001));
       *pte=(page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/* 分配pg_cnt个页空间，成功则返回起始虚拟地址，失败则返回NULL */
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt){
    ASSERT(pg_cnt>0 && pg_cnt<3840);
    /************ malloc_page 的原理是三个动作的合成：**********
     * 1.通过vaddr_get在虚拟地址内存池中的申请虚拟地址
     * 2.通过palloc在物理内存池中申请物理页
     * 3.通过page_table_add将以上得到的虚拟地址和物理地址在页表中完成映射
     **********************************************************/
    void* vaddr_start=vaddr_get(pf,pg_cnt);
    if (vaddr_start==NULL){
        return NULL;
    }

    uint32_t vaddr=(uint32_t)vaddr_start,cnt=pg_cnt;
//现在好像只能选择kernel_pool，因为user_pool没有初始化
    struct pool* mem_pool=(pf & PF_KERNEL)?&kernel_pool:&user_pool;

    /* 因为虚拟地址是连续的，但物理地址可以是不连续的，所以逐个做映射 */
//所以物理内存要逐个申请
    while (cnt-->0){
    void* page_phyaddr=palloc(mem_pool);
    if (page_phyaddr==NULL){    // 申请物理内存失败，将已申请的虚拟地址和物理页全部回滚，在将来完成内存回收时再补充
        return NULL;
    }
    page_table_add((void*)vaddr,page_phyaddr);   // 在页表中做映射
    vaddr+=PG_SIZE;   // 下一个虚拟页
    }
    return vaddr_start;
}
//上面写的函数基本上都是为他服务的
/* 从内核物理内存池中申请1页内存，成功则返回其虚拟地址，失败则返回NULL */
void* get_kernel_pages(uint32_t pg_cnt){
    void* vaddr=malloc_page(PF_KERNEL,pg_cnt);
    if (vaddr!=NULL){   // 若分配的地址不为空，将页框清0
    memset(vaddr,0,pg_cnt*PG_SIZE);
    }
    return vaddr;
}

/* 初始化内存池 */
void mem_pool_init(uint32_t all_mem){
    put_str("   mem_pool_init start\n");
    uint32_t page_table_size=PG_SIZE*256;

    uint32_t used_mem=page_table_size+0x100000;
    uint32_t free_mem=all_mem-used_mem;
    uint16_t all_free_pages=free_mem/PG_SIZE;

    uint16_t kernel_free_pages=all_free_pages/2;
    uint16_t user_free_pages=all_free_pages-kernel_free_pages;

    /* 为简化位图操作，余数不处理，不用考虑越界检查，但会丢失部分内存 */
    uint32_t kbm_length=kernel_free_pages/8;   // kernel bitmap的长度。1位管理1页，单位为字节
    uint32_t ubm_length=user_free_pages/8;   // user bitmap的长度

    uint32_t kp_start=used_mem;   // KernelLPool，内核内存池的起始地址
    uint32_t up_start=kp_start+kernel_free_pages*PG_SIZE;   // UserPool，用户内存池的起始地址

    kernel_pool.phy_addr_start=kp_start;
    user_pool.phy_addr_start=up_start;

    kernel_pool.pool_size=kernel_free_pages*PG_SIZE;
    user_pool.pool_size=user_free_pages*PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len=kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len=ubm_length;


    /************** 内核内存池和用户内存池位图 ***************
     * 位图是全局的数据，长度不固定。
     * 全局或静态的数组需要在编译时知道其长度，
     * 而我们需要根据总内存大小算出需要多少字节，
     * 所以改为指定一块内存来生成位图。
     * ******************************************************/
    // 内核使用的最高地址是0xc009f000，这是主线程的站地址
    // 32MB内存占用的位图是2KB，内核内存池位图先定在MEM_BITMAP_BASE(0xc009a000)处
    kernel_pool.pool_bitmap.bits=(void*)MEM_BITMAP_BASE;
//这也就说明了内核位图和用户位图都存放在内核虚拟地址空间中
    /* 用户内存池的位图紧跟在内核内存池位图之后 */
    user_pool.pool_bitmap.bits=(void*)(MEM_BITMAP_BASE+kbm_length);

    /***************** 输出内存池信息 ***********************/
    put_str("   kernel_pool_bitmap_start:");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_str("   kernel_pool_phy_addr_start:");
    put_int((int)kernel_pool.phy_addr_start);
    put_str("\n");
    put_str("   user_pool_bitmap_start:");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str("   user_pool_phy_addr_strar:");
    put_int((int)user_pool.phy_addr_start);
    put_str("\n");

    /*将位图置0*/
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    /* 下面初始化内核虚拟地址位图，按实际物理内存大小生成数组 */
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len=kbm_length;
    // 用于维护内核堆的虚拟地址，所以要和内核内存池大小一致

    /* 位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外 */
    kernel_vaddr.vaddr_bitmap.bits=(void*)(MEM_BITMAP_BASE+kbm_length+ubm_length);

    kernel_vaddr.vaddr_start=K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    put_str("   mem_pool_init done\n");
}

/* 内存管理部分初始化入口 */
void mem_init(){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total=(*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);   // 初始化内存池
    put_str("mem_init done\n");
}
