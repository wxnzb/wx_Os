#include "timer.h"
#include "io.h"
#include "print.h"
#include "../thread/thread.h"
#include "../kernal/interrupt.h"
#include "debug.h"

#define IRQ0_FREQUENCY             100
#define INPUT_FREQUENCY         1193180
#define COUNTER0_VALUE        INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT        0X40
#define COUNTER0_NO         0
#define COUNTER_MODE        2
#define READ_WRITE_LATCH    3
#define PIT_COUNTROL_PORT    0x43

uint32_t ticks;     // ticks是内核自中断开启以来总共的嘀嗒数

static void frequency_set(uint8_t counter_port,\
              uint8_t counter_no,\
              uint8_t rwl,\
              uint8_t counter_mode,\
              uint16_t counter_value)
{
    outb(PIT_COUNTROL_PORT,(uint8_t) (counter_no<<6|rwl<<4|counter_mode<<1));
    outb(counter_port,(uint8_t)counter_value);
    outb(counter_port,(uint8_t)counter_value>>8);
}

static void intr_timer_handler(void){
    struct task_struct* cur_thread=running_thread();

    ASSERT(cur_thread->stack_magic==0x19870916);   // 检查栈是否溢出
//所以中断处理程序被cpu执行一次，他的elapsed_ticks就只+1？？？？？
    cur_thread->elapsed_ticks++;   // 记录此线程占用的cpu时间
//为啥是内核态和用户态总共的嘀嗒数？？？？？？？？？
    ++ticks;                       // 从内核第一次处理时间中断后开始至今的嘀嗒数，内核态和用户态总共的嘀嗒数

    if (cur_thread->ticks==0){
    schedule();
    }else{
        --cur_thread->ticks;
    }
}

/* 初始化PIT8253 */
void timer_init(void)
{
    put_str("timer_init start!\n");
    frequency_set(COUNTER0_PORT,\
          COUNTER0_NO,\
          READ_WRITE_LATCH,\
          COUNTER_MODE,\
          COUNTER0_VALUE);
    register_handler(0x20,intr_timer_handler);
    put_str("timer_init done!\n");
}