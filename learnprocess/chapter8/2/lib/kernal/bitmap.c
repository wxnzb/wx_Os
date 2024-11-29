//这里最好奇的就是字节以及字节中的位的遍历方向
#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"
//看来看去太麻烦了，他就长这样奥
// struct bitmap {
//    uint32_t btmp_bytes_len;
// /* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
//    uint8_t* bits;
// };
/* 将位图btmp初始化 */
void bitmap_init(struct bitmap* btmp){
//从btmp->bits开始的btmp->btmp_bytes_len个字节都初始化为0
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}

/* 判断bit_idx位是否为1，若为1，则返回true，否则返回false */
bool bitmap_scan_test(struct bitmap* btmp,uint32_t bit_idx){
    uint32_t byte_idx=bit_idx/8;   // 向下取整用于索引数组下标
    uint32_t bit_off=bit_idx%8;   // 取余用于索引数组内的值
//得到某个位，要判断该位里面的某个的状态，你就要将1享有偏移他的与数
//？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
//btmp->bits[byte_idx]代表8bit，比如01000000,bit_idx=1,bit_off=1,0100000000&10??这到底是咋判断的呀
//原来如此：bit_idx 是从右向左计数的！！！，因此他本来就是0,我还以为是从左往右，乐
    return (btmp->bits[byte_idx]&(BITMAP_MASK<<bit_off));
}

/* 在位图中申请连续cnt个位，成功则返回其起始位下标，失败则返回-1 */
int bitmap_scan(struct bitmap* btmp,uint32_t cnt){
    uint32_t idx_byte=0;   // 用于记录空闲位所在的字节
    /* 逐字节比较，蛮力法 */
//从头开始扫描，找到字节数组中有0的
    while ((0xff==btmp->bits[idx_byte]) && (idx_byte<btmp->btmp_bytes_len)){
// 1表示已分配，若为0xff，说明该字节内已无空闲位，向下一字节继续找
    idx_byte++;
    }
//这里要再次判断的原因是上面是&&，只要一个为0就跳出了
//要是已经出了范围，ASSERT就会打印错误信息并通过while(1)悬停
    ASSERT(idx_byte<btmp->btmp_bytes_len);
    if (idx_byte==btmp->btmp_bytes_len) {   // 若内存池中找不到可用空间
        return -1;
    }

    /* 若在位图数组范围内的某字节内找到了空闲位
     * 在该字节内逐位比对，返回空闲位索引 */
    int idx_bit=0;
    /* 和btmp->bits[idx_byte]这个字节逐位比对 */
//不是，我咋感觉(btmp->bits[idx_byte]）这个不太需要，前面不是已经找到了吗
//ok，再看会了，他就是从右向左找第一个为0的位置
    while ((uint8_t)(BITMAP_MASK<<idx_bit)&(btmp->bits[idx_byte])){
    idx_bit++;
    }
//很好奇这个，idx_byte他是从左向右遍历，idx_bit但是他是从右向左遍历，好乱？？？？？？？？？？？？？？？？？？？？？？？
    int bit_idx_start=idx_byte*8+idx_bit;   // 空闲位在位图内的下标
    if (cnt==1){
        return bit_idx_start;
    }
    uint32_t bit_left=(btmp->btmp_bytes_len*8-bit_idx_start);
    // 记录还有多少位可以判断
    uint32_t next_bit=bit_idx_start+1;
    uint32_t count=1;   // 用于记录找到的空闲位的个数

    bit_idx_start=-1;
    while (bit_left-->0){
    if (!bitmap_scan_test(btmp,next_bit)){
        count++;
    }else{
        count=0;
    }
//bit_idx_start=next_bit-cnt+1;回退到第一个为0的位置
    if (count==cnt){
        bit_idx_start=next_bit-cnt+1;
        break;
    }
    next_bit++;
    }
    return bit_idx_start;
}

/* 将位图btmp的bit_idx位设置为value */
void bitmap_set(struct bitmap* btmp,uint32_t bit_idx,int8_t value){
    ASSERT((value==0) || (value==1));
    uint32_t byte_idx=bit_idx/8;
    uint32_t bit_off=bit_idx%8;

    /* 将1移位进行操作 */
    if (value){
    btmp->bits[byte_idx]|=(BITMAP_MASK<<bit_off);
    }
    else{
    btmp->bits[byte_idx]&=~(BITMAP_MASK<<bit_off);
    }
}