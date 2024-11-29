//这段代码的主要作用是打印出错误信息，并使程序悬停
#include "debug.h"
#include "print.h"
#include "interrupt.h"

/* 打印文件名、行号、函数名、条件并使程序悬停 */
void panic_spin(char* filename,     \
        int line,            \
        const char* func ,     \
        const char* condition) \
{
    //先要关闭中断，不然害怕影响屏幕输出
    intr_disable();   // 因为有时候会单独调用panic_spin，所以在此处关中断
    put_str("\n\n\n!!!!! error !!!!!\n");
    put_str("filename:");put_str(filename);put_str("\n");
    put_str("ilne:0x");put_int(line);put_str("\n");
    put_str("function:");put_str((char*)func);put_str("\n");
    put_str("condition:");put_str((char*)condition);put_str("\n");
    while (1);
}