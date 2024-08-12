// int s = socket(AF_INET, SOCK_STREAM, 0);  
// bind(s, ...)
// listen(s, ...)
 
// int fds[] =  存放需要监听的socket
 
// while(1){
//     int n = select(..., fds, ...)
//     for(int i=0; i < fds.count; i++){
//         if(FD_ISSET(fds[i], ...)){
//             //fds[i]的数据处理
//         }
//     }
// }