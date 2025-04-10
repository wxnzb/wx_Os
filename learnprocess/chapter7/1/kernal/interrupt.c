#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"
// 这一行是我自己加的，真不错
#include "print.h"

#define PIC_M_CTRL 0x20           // 主片的控制端口是0x20
#define PIC_M_DATA 0x21           // 主片的数据端口是0x21
#define PIC_S_CTRL 0xa0           // 从片的控制端口是0xa0
#define PIC_S_DATA 0xa1           // 从片的数据端口是0xa1

#define IDT_DESC_CNT 0x21         // 目前总共支持的中断数

/*中断门描述符结构体*/
struct gate_desc{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t  dcount;               // 此项为双字计数字段，是门描述符的第4字节，固定值
    uint8_t  attribute;            // 属性值
    uint16_t func_offset_high_word;
};

// 静态函数声明，非必须
static void make_idt_desc(struct gate_desc* p_gdesc,uint8_t attr,intr_handler function);
static struct  gate_desc idt[IDT_DESC_CNT];   // IDT是中断描述符表，本质上就是个中断描述符数组

extern intr_handler intr_entry_table[IDT_DESC_CNT];   // 声明引用定义在kernel.S中的中断处理函数入口数组

/* 初始化可编程中断控制器8259A */
static void pic_init(void) {

    /*初始化主片*/
    outb(PIC_M_CTRL,0x11);
    outb(PIC_M_DATA,0x20);
    outb(PIC_M_DATA,0x04);
    outb(PIC_M_DATA,0x01);

    /*初始化从片*/
    outb(PIC_S_CTRL,0x11);
    outb(PIC_S_DATA,0x28);
    outb(PIC_S_DATA,0x02);
    outb(PIC_S_DATA,0x01);

    /*打开主片上IR0，也就是目前只接受时钟产生的中断*/
    outb(PIC_M_DATA,0xfe);
    outb(PIC_S_DATA,0xff);

    put_str("   pic_init done\n");
}

/*创建中断门描述符*/
static void make_idt_desc(struct gate_desc* p_gdesc,uint8_t attr,intr_handler function){
    p_gdesc->func_offset_low_word=(uint32_t)function & 0x0000ffff;
    p_gdesc->selector=SELECTOR_K_CODE;   // 中断后进入内核代码
    p_gdesc->dcount=0;
    p_gdesc->attribute=attr;
    p_gdesc->func_offset_high_word=((uint32_t)function & 0xffff0000)>>16;
}
//浅浅好奇一下，为啥intr_entry_table[i]这个在这之前就已经初始化了，就是那个打印"interrupt occur!"
/*初始化中断描述符表*/
static void idt_desc_init(void){
    int i;
    for (i=0;i<IDT_DESC_CNT;++i){
        make_idt_desc(&idt[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
    }
    put_str("   idt_desc_init done\n");
}

/*完成有关中断的所有初始化工作*/
void idt_init(){
    put_str("idt_init start\n");
    idt_desc_init();               // 初始化中断描述符表
    pic_init();                    // 初始化PIC(8259A)

    /*加载idt*/
    uint64_t idt_operand=((sizeof(idt)-1)|((uint64_t)(uint32_t)idt<<16));
    asm volatile("lidt %0"::"m"(idt_operand));
    put_str("idt_init done\n");
}