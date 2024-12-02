#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"   //相对路径
#include "memory.h"
#include "thread.h"

/*负责初始化所有模块*/
void init_all(){
    put_str("init_all\n");
    idt_init();   // 初始化中断
    timer_init(); // 初始化PIT
    mem_init();
    thread_init();
}