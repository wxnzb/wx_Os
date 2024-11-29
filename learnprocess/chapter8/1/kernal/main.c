#include "print.h"
#include "init.h"
#include "debug.h"
int main(void)
{
  put_str("Welcome,\nI am kernel!\n");
  init_all();
  ASSERT(1==2);//这是第一次加的
  //asm volatile("sti");   // 为演示中断处理，在此临时开中断
  while(1);
  return 0;
}

