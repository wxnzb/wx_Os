//用宏判断断言是否成立，不成立就调用相应的函数
#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H
//下面这个是__KERNEL_DEBUG_H这个的内容
void panic_spin(char* filename,int line,const char* func,const char* condition);

/**************************** __VA_ARGS__ *******************************
 * __VA_ARGS__是预处理器所支持的专用标识符。
 * 代表所有与省略号相对应的参数。
 * "..."表示定义的宏，其参数可变。*/
//__FILE__,__LINE__,__func__那这些都是计算机自己生成和处理的
#define PANIC(...) panic_spin(__FILE__,__LINE__,__func__,__VA_ARGS__)
/***********************************************************************/
//NOEBUG(No Debug)表示关闭条时功能
#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
    #define ASSERT(CONDITION) \
    if (CONDITION){} \
    /* 符号#让编译器将宏的参数转化为字符串字面量 */ \
    else {PANIC(#CONDITION);}
#endif /* __NDEBUG */

#endif /* __KERNEL_DEBUG_H */