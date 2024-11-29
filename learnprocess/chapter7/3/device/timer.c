//初始化一个定时器
#include "timer.h"
#include "io.h"
#include "print.h"
//操作系统的时钟中断频率
#define IRQ0_FREQUENCY             100   
//定时器的输入时钟频率，也就是 PIT 的计数器频率
#define INPUT_FREQUENCY         1193180
//计算定时器计数器的值，表示将输入时钟频率转换为所需的输出频率
#define COUNTER0_VALUE        INPUT_FREQUENCY / IRQ0_FREQUENCY
//计数器0的端口地址（0x40），这是PIT中计数器0的数据端口。
#define COUNTER0_PORT        0X40
//计数器编号，这里是计数器0
#define COUNTER0_NO         0
//计数器的工作模式。2 表示定时器的工作模式是 "rate generator"（速率生成器模式）
#define COUNTER_MODE        2
//控制字节的读写模式，这里是 3，表示读/写锁存器模式。
#define READ_WRITE_LATCH    3
//PIT 控制端口（0x43），用于发送控制命令到 PIT。
#define PIT_COUNTROL_PORT    0x43
//配置计数器的频率，将属性配置好后发送给PIC的控制寄存器
static void frequency_set(uint8_t counter_port,\
              uint8_t counter_no,\
              uint8_t rwl,\
              uint8_t counter_mode,\
              uint16_t counter_value)
{
    outb(PIT_COUNTROL_PORT,(uint8_t) (counter_no<<6|rwl<<4|counter_mode<<1));
    //这里向0计数器的寄存器写入数字为了设计定时器的中断频率
    outb(counter_port,(uint8_t)counter_value);
    outb(counter_port,(uint8_t)counter_value>>8);
}

void timer_init(void)
{
    put_str("timer_init start!\n");
    frequency_set(COUNTER0_PORT,\
          COUNTER0_NO,\
          READ_WRITE_LATCH,\
          COUNTER_MODE,\
          COUNTER0_VALUE);
    put_str("timer_init done!\n");
}