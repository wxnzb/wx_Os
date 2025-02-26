#include "sync.h"
#include "stdint.h"
#include "list.h"
#include "thread.h"
#include "debug.h"
#include "interrupt.h"
/* 初始化信号量 */
void sema_init(struct semaphore* psema,uint8_t value){
    psema->value=value;
    list_init(&psema->waiters);
}

/* 初始化锁 */
void lock_init(struct lock* plock){
    plock->holder=NULL;
    sema_init(&plock->semaphore,1);   // 信号量初值为1
    plock->holder_repeat_nr=0;
}

/* 信号量dowm操作 */
//你要运行这个线程，想要拿到信号量，要是value为0，表示有人拿着，你就得等着，要是value为1，表示没人拿着，你就把value减1，然后运行这个线程,所以这个的前提是他是二值信号量吗？？？？？？？？？
void sema_down(struct semaphore* psema){
    /* 关中断来保证原子操作 */
    enum intr_status old_status=intr_disable();
    while (psema->value==0){   // 若value为0，表示已被别的thread持有
//还是和之前一样，ASSET若判断了，那会悬停呀，怎么还会运行后面的代码？？？？？？？
//这行代码的作用是确保当前线程没有已经在等待队列中。如果当前线程已经在等待队列中，表示它已经执行过sema_down操作并被阻塞了，原来如此
       ASSERT(!elem_find(&psema->waiters,&running_thread()->general_tag));
    /* 当前线程不应再已在信号量的waiters队列中 */
      if (elem_find(&psema->waiters,&running_thread()->general_tag)){
        PANIC("sema_down: thread blocked has been in waiters_list\n");
      }
    /* 若信号量的值等于0，则当前线程把自己加入等待队列 */
      list_append(&psema->waiters,&running_thread()->general_tag);
       thread_block(TASK_BLOCKED);   // 阻塞线程，直到被唤醒
    }
    /* 若value为1或被唤醒后，会执行以下代码，也就是获得了锁  */
    --psema->value;
//这里感觉不需要判断呀，要是不为0就为1,为1-1不就等于0吗
    ASSERT(psema->value==0);
    /* 恢复之前的状态 */
    intr_set_status(old_status);
}

/* 信号量的up操作 */
//也就是释放锁的操作
void sema_up(struct semaphore* psema){
    /* 关中断，保证原子操作 */
    enum intr_status old_status=intr_disable();
    ASSERT(psema->value==0);
//因为他是释放锁，等待信号量的双向链表不为空，那就取出第一个
    if (!list_empty(&psema->waiters)){
      struct task_struct* thread_blocked=elem2entry(struct task_struct,general_tag,list_pop(&psema->waiters));
      thread_unblock(thread_blocked);
    }
//要是为空，那就value+1
//那为啥这里和上面都没有else呢？？？？？？？？好问题！！
//上面要是if为真，就将改线程阻塞了，后面的代码还是不会运行
//这里要是if为真，他就去执行thread_blocked这个线程去了，就不回来了
    ++psema->value;
    ASSERT(psema->value==1);
    /* 恢复之前的状态 */
    intr_set_status(old_status);
}

/* 获取锁 */
void lock_acquire(struct lock* plock){
//如果持有锁的pcb不等于当前线程，那么plock->holder可能是NULL，那么就可以运行你想运行的这个线程，要是不为NULL，那就是别的线程持有锁，你就得等待，等待别人释放锁
    if (plock->holder!=running_thread()){
      sema_down(&plock->semaphore);   // 对信号P操作，原子操作
      plock->holder=running_thread();
      ASSERT(plock->holder_repeat_nr==0);
      plock->holder_repeat_nr=1;
    }else{
       ++plock->holder_repeat_nr;
    }
}

/* 释放锁 */
void lock_release(struct lock* plock){
//你要释放他，首先你得拥有他，嘻嘻
    ASSERT(plock->holder==running_thread());
    if (plock->holder_repeat_nr>1){
        --plock->holder_repeat_nr;
      return;
    }
    ASSERT(plock->holder_repeat_nr==1);

    plock->holder=NULL;               // 把锁的持有者置空放在V操作之前
    plock->holder_repeat_nr=0;
    sema_up(&plock->semaphore);       // 信号量的V操作，也是原子操作
}