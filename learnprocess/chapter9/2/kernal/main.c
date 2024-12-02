#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void) {
   put_str("Welcome,\nI am kernel!\n");
   init_all();

   thread_start("k_thread_a",31,k_thread_a,"argA ");
   thread_start("k_thread_b",31,k_thread_b,"argB ");

   intr_enable();

   while(1){
       intr_disable();//代表锁的作用，不加就运行出错
       put_str("Main ");
       intr_enable();
   }
   return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void* arg){
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para=arg;
    while (1){
        intr_disable();
        put_str(para);
        intr_enable();
    }
}

void k_thread_b(void* arg){
    char* para=arg;
    while (1){
        intr_disable();
        put_str(para);
        intr_enable();
    }
}