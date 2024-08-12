#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

int main(void)
{
   int epfd,nfds;
   struct epoll_event ev,events[5]; //ev用于注册事件，数组用于返回要处理的事件
   epfd = epoll_create(1); //只需要监听一个描述符
   ev.data.fd = STDIN_FILENO;//表示要监听的文件描述符——标准输入
   ev.events = EPOLLIN ; //监听读状态同时设置ET模式  EPOLLET
   epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev); //注册epoll事件,将标准输入文件描述符加入到 epoll 实例中进行监听
   for(;;)
   {
       nfds = epoll_wait(epfd, events, 5, -1);//等待事件发生，当标准输入有可读事件发生时，会返回就绪的事件列表
       for(int i = 0; i < nfds; i++)
        {
             if(events[i].data.fd==STDIN_FILENO)//判断返回的事件是否是标准输入的事件
             
             {
                char buf[1024]={0};
                read(STDIN_FILENO,buf,sizeof(buf));
                printf("welcome to epoll's word!\n");
             }
            }
    }
}