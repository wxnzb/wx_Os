#include "memory.h"
#include "stdint.h"
#include "print.h"

//一页的大小
#define PG_SIZE 4096

/********************** 位图地址 **********************/
#define MEM_BITMAP_BASE 0xc009a000

/* 0xc0000000是内核从虚拟地址3G起。0x100000意指跨过低端1MB内存，使虚拟地址在逻辑上连续 */
#define K_HEAP_START 0xc0100000

//-----------------------搬到这来，不然不太好看
// struct virtual_addr{
//     struct bitmap vaddr_bitmap;   // 虚拟地址用到了位图结构
//     uint32_t vaddr_start;   // 虚拟地址起始地址
// };
//-----------------------
/* 内存池结构，生成两个实例用于管理内核内存池和用户内存池 */
struct pool{
    struct bitmap pool_bitmap;   // 本内存池用到的位图结构，用于管理物理内存
    uint32_t phy_addr_start;   // 本内存池所管理物理内存的起始地址
    uint32_t pool_size;   // 本内存池字节容量
};

struct pool kernel_pool,user_pool;   // 生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr;   // 此结构用来给内核分配虚拟地址

/* 初始化内存池 */
static void mem_pool_init(uint32_t all_mem){
    put_str("   mem_pool_init start\n");
//我去，果然，这里提到的地址都是物理地址
//？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
//4kb*256=1mb,也就是0x000000-0x100000这段物理地址，虽然我不知道这是在干啥？？？？
    uint32_t page_table_size=PG_SIZE*256;
//为啥又要+1mb??????????????????????//
    uint32_t used_mem=page_table_size+0x100000;
    uint32_t free_mem=all_mem-used_mem;
    uint16_t all_free_pages=free_mem/PG_SIZE;

    uint16_t kernel_free_pages=all_free_pages/2;
    uint16_t user_free_pages=all_free_pages-kernel_free_pages;

    /* 为简化位图操作，余数不处理，不用考虑越界检查，但会丢失部分内存 */
    uint32_t kbm_length=kernel_free_pages/8;   // kernel bitmap的长度。1位管理1页，单位为字节
    uint32_t ubm_length=user_free_pages/8;   // user bitmap的长度
//虽然内核态的虚拟地址在3-4GB，但是内核内存池的虚拟地址在用户的前面
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
//那内核内存池和用户内存池到底用的是物理内存还是虚拟内存阿？？？？？？？？？？？？？？？？？？？？
    /* 位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外 */
    kernel_vaddr.vaddr_bitmap.bits=(void*)(MEM_BITMAP_BASE+kbm_length+ubm_length);
//为啥要指向K_HEAP_START？？？？？？？？？？？？？？？？？？？？？？
    kernel_vaddr.vaddr_start=K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    put_str("   mem_pool_init done\n");
}

/* 内存管理部分初始化入口 */
void mem_init(){
    put_str("mem_init start\n");
//!!!0xb00是地址，他指向了物理内存的总大小，你看嘛，对他进行解引用了呀，第五章有说他最大支持4GB
    uint32_t mem_bytes_total=(*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);   // 初始化内存池
    put_str("mem_init done\n");
}
//？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
//运行输出结果： c009A000  200000 c009A1E0 1100000
//算一下最大物理内存0x1100000-0x200000=0x900000(这是内核占的物理地址)*2=0x1800000(这是内核和用户的物理地址，因为他两是平分的)+0x200000=0x1A00000(0xb00处存储的最大物理地址)=25 MB？？？这么小吗
//我要崩溃了
