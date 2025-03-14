#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"

// 定义一种叫thread_fun的函数类型，该类型返回值是空，参数是一个地址(这个地址用来指向自己的参数)。
// 这样定义，这个类型就能够具有很大的通用性，很多函数都是这个类型
// 自定义通用函数类型，它将在很多线程函数中作为形参类型
typedef void thread_func(void *);

/* 进程或线程的状态 */
enum task_status
{
    TASK_RUNNING, // 运行
    TASK_READY,   // 就绪
    TASK_BLOCKED, // 阻塞
    TASK_WAITING, // 等待
    TASK_HANGING, // 挂起
    TASK_DIED     // 死亡
};

/***********   中断栈intr_stack   ***********
 * 此结构用于中断发生时保护程序(线程或进程)的上下文环境:
 * 进程或线程被外部中断或软中断打断时,会按照此结构压入上下文
 * 寄存器,  intr_exit中的出栈操作是此结构的逆操作
 * 此栈在线程自己的内核栈中位置固定,所在页的最顶端
 ********************************************/
struct intr_stack
{
    uint32_t vec_no; // kernel.S 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* 以下由cpu从低特权级进入高特权级时压入 */
    uint32_t err_code; // err_code会被压入在eip之后 6
    void (*eip)(void); // 旧EIP 5
    uint32_t cs;       // 旧代码段 4
    uint32_t eflags;   // EFLAGS寄存器 3
    void *esp;         // 旧栈指针 2
    uint32_t ss;       // 旧栈 1
};

/***********  线程栈thread_stack  ***********
 * 线程自己的栈,用于存储线程中待执行的函数
 //那你为啥把他放在内核栈的第二，第一是上面那个中断结构体？？？？？？？？？还说什么不固定
 * 此结构在线程自己的内核栈中位置不固定,
 //这是什么玩意？？？？？？？？
 * 用在switch_to时保存线程环境。
 * 实际位置取决于实际运行情况。
 ******************************************/
struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    // 这个位置会放一个名叫eip，返回void的函数指针(*epi的*决定了这是个指针)，
    // 该函数传入的参数是一个thread_func类型的函数指针与函数的参数地址
//为啥第一次执行就是kernel_thread？？？？？？？？？？？？？？？
    /* 线程第一次执行时，eip 指向待调用的函数 kernel_thread
    其他时候，eip 是指向 switch_to 的返回地址*/
    void (*eip)(thread_func *func, void *func_arg);

    /***** 以下仅供第一次被调度上 cpu 时使用 ****/
//thread_start？？？为啥要用call？？？？？？？？？？
    // 以下三条是模仿call进入thread_start执行的栈内布局构建的，call进入就会压入参数与返回地址，因为我们是ret进入kernel_thread执行的
    // 要想让kernel_thread正常执行，就必须人为给它造返回地址，参数
    void(*unused_retaddr); // 参数 unused_ret 只为占位置充数为返回地址
    thread_func *function; // Kernel_thread运行所需要的函数地址
    void *func_arg;        // Kernel_thread运行所需要的参数地址
};

/* 进程或线程的pcb,程序控制块, 此结构体用于存储线程的管理信息*/
struct task_struct
{
//感觉这个不是很懂？？？？？？？？
    uint32_t *self_kstack; // 用于存储线程的栈顶位置，栈顶放着线程要用到的运行信息
    enum task_status status;
    uint8_t priority;     // 线程优先级
    char name[16];        // 用于存储自己的线程的名字
//感觉这个不是很懂？？？？？？？？
    uint32_t stack_magic; // 如果线程的栈无限生长，总会覆盖地pcb的信息，那么需要定义个边界数来检测是否栈已经到了PCB的边界
};

void thread_create(struct task_struct *pthread, thread_func function, void *func_arg);
void init_thread(struct task_struct *pthread, char *name, int prio);
struct task_struct *thread_start(char *name, int prio, thread_func function, void *func_arg);

#endif