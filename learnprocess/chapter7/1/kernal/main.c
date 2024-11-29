#include "print.h"
#include "init.h"
void main(void){
    put_str("i am wuxi\n");
    init_all();
    asm volatile("sti");
    while(1);
}