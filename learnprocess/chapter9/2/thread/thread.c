#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "print.h"
#include "debug.h"

#define PG_SIZE 4096

struct task_struct* main_thread;   // 主线程PCB
struct list thread_ready_list;     // 就绪队列
struct list thread_all_list;       // 所有任务队列
static struct list_elem* thread_tag;// 用于保存队列中的线程结点

extern void switch_to(struct task_struct* cur,struct task_struct* next);

/* 获取当前pcb指针 */
struct task_struct* running_thread(){
    uint32_t esp;
//将esp寄存器赋给esp变量
    asm("mov %%esp,%0":"=g"(esp));
    /* 取asm整数部分，即pcb起始地址 */
//那为啥pcb的地址存储在esp寄存器的高20位？？？？？？？
//因为在一页中，pcb的起始地址为这一页的起始地址，esp指向的栈顶位于这一页的末尾,esp存储的是栈顶地址，栈顶和pcb在一个页，高20存储的是pte(页表项)的地址，因pcb是这一页起始，所以想等
//那么现在还有一个问题，栈是由高地址向低地址增长的，pcb栈有空间是向上增长的，虽然他占得很小，但怎样确定他两在这一页中不会出现重叠？？？？？？？？？？
    return (struct task_struct*)(esp & 0xfffff000);
}

/* 由kernel_thread去执行function(func_arg) */
void kernel_thread(thread_func* function,void* func_arg){
//为啥上一个1没有开中断而这个有呢？？？？？？？？？
    /* 执行function前要开中断，避免后面的时钟中断被屏蔽，而无法调度其它进程 */
    intr_enable();
    function(func_arg);
}

/* 初始化线程栈thread_stack，将待执行的函数和参数方法到thread_stack中相应的位置 */
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg){
    /* 先预留中断使用栈的空间，可见thread.h中定义的结构 */
//那第一个地址pthread->self_kstack是从哪来的？？？？？？？？？？
    pthread->self_kstack-=sizeof(struct intr_stack);

    /* 再留出线程栈空间，可见thread.h中定义 */
    pthread->self_kstack-=sizeof(struct thread_stack);
    struct thread_stack* kthread_stack=(struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip=kernel_thread;
    kthread_stack->function=function;
    kthread_stack->func_arg=func_arg;
    kthread_stack->ebp=kthread_stack->ebx=kthread_stack->esi=kthread_stack->edi=0;
}

/* 初始化线程基本信息 */
void init_thread(struct task_struct* pthread,char* name,int prio){
    memset(pthread,0,sizeof(*pthread));
    strcpy(pthread->name,name);
//main_thread这个啥时有被定义吗？？？
    if (pthread==main_thread){
    /* 由于把main函数也封装成一个线程，并且它一直是运行的，故将其直接设为TASK_RUNNING */
    pthread->status=TASK_RUNNING;
    }else{
    pthread->status=TASK_READY;
    }

    /* self_kstack是线程自己在内核态下使用的栈顶地址 */
    pthread->self_kstack=(uint32_t*)((uint32_t)pthread+PG_SIZE);
    pthread->priority=prio;
    pthread->ticks=prio;
    pthread->elapsed_ticks=0;
    pthread->pgdir=NULL;
    pthread->stack_magic=0x19870916;   // 自定义魔数
}

//这个函数主要是创建线程pcb,线程栈以及将pcb结构体中的general_tag加入到就绪队列中
/* 创建一个优先级为prio的线程，线程名为name，线程所执行的函数是funciton(func_arg) */
struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg){
    /* pcb都位于内核空间，包括用户进程pcb也是在内核空间 */
    struct task_struct* thread=get_kernel_pages(1);

    init_thread(thread,name,prio);//pcb
    thread_create(thread,function,func_arg);//线程栈

    /* 确保之前不再队列中 */
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));
    /* 加入就绪队列 */
    list_append(&thread_ready_list,&thread->general_tag);

    /* 确保之前不再队列中 */
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    list_append(&thread_all_list,&thread->all_list_tag);

    return thread;
}

/* 将kernel中的main函数完善为主线程 */
static void make_main_thread(void){
    /* 因为main线程早已运行，咱们在loader.S中进入内核时的mov esp,0xc009f000,
     * 就是为其预留pcb的，因此pcb地址为0xc009e000，不需要勇敢get_kernel_page另分配一页 */
//你既然说mov esp,0xc009f000，那么esp中存放的根本就不是pcb的地址阿，还要减去4kb阿
    main_thread=running_thread();
    init_thread(main_thread,"main",31);

    /* main函数是当前线程，当前线程不在thread_ready_list中，
     * 所以只将其加在thead_all_list中 */
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
}

/* 实现任务调度 */
void schedule(){
//为什么要在关中断的情况下执行schedule函数？???
//为了保证任务调度中的原子性，防止任务调度过程中被中断打断
    ASSERT(intr_get_status()==INTR_OFF);
//他通过running_thread获得的是pcb，但是esp之前不是是main主线程栈顶地址吗，所以cur不都是main的pcb吗，好奇怪？？？？？？？？？？？？？/
//我猜是因为esp永远指向但前正在运行线程的栈顶，所以cur指向的是当前正在运行的线程的pcb
    struct task_struct* cur=running_thread();
//为啥等于正在运行就代表这个线程在cpu上时间片用完了？？？？？？？？？
    if (cur->status==TASK_RUNNING){
    // 若此线程只是cpu时间片到了，将其加入就绪队列队尾
    ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
    list_append(&thread_ready_list,&cur->general_tag);
    cur->ticks=cur->priority;
    // 重新将当前进程的ticks重置为priority
    cur->status=TASK_READY;
    }else{
    /* 若此线程需要某事件发生后才能继续上cpu运行，
     * 不需要将其加入队列，因为当前不在就绪队列中 */
    }

    ASSERT(!list_empty(&thread_ready_list));
//thread_tag的作用是啥？？？？？？？？？
    thread_tag=NULL;   // thread_tag清空
    /* 将thread_ready_list队列中的第一个就绪线程弹出，准备将其调度上cpu */
    thread_tag=list_pop(&thread_ready_list);
    struct task_struct* next=elem2entry(struct task_struct,general_tag,thread_tag);
    next->status=TASK_RUNNING;
    switch_to(cur,next);
}

/* 初始化线程环境 */
void thread_init(void){
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    /* 将当前main函数创建为线程 */
    make_main_thread();
    put_str("thread_init done\n");
}