;初始化中断号以及所对应的中断程序
[bits 32]
%define ERROR_CODE nop
%define ZERO push 0

extern put_str              ;声明外部函数

section .data
intr_str db "interrupt occur!",0xa,0
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text
intr%1entry:                ;每个中断处理程序都要压入中断向量号，一个中断类型一个中断处理程序
   %2
   push intr_str
   call put_str
   add esp,4                ;跳过参数

   ;如果是从片上进入的中断，除了往从片上发送EOI外，还要往主片上发送EOI
   mov al,0x20              ;中断结束命令EOI
   out 0xa0,al              ;向从片发送
   out 0x20,al              ;向主片发送


   add esp,4                ;跨过error_code
   iret                     ;从中断返回

section .data
   dd intr%1entry           ;存储各个中断入口程序的地址，形成intr_entry_table数组
%endmacro

VECTOR 0x0 ,ZERO
VECTOR 0x1 ,ZERO
VECTOR 0x2 ,ZERO
VECTOR 0x3 ,ZERO
VECTOR 0x4 ,ZERO
VECTOR 0x5 ,ZERO
VECTOR 0x6 ,ZERO
VECTOR 0x7 ,ZERO
VECTOR 0x8 ,ERROR_CODE
VECTOR 0x9 ,ZERO
VECTOR 0xA ,ERROR_CODE
VECTOR 0xB ,ERROR_CODE
VECTOR 0xC ,ERROR_CODE
VECTOR 0xD ,ERROR_CODE
VECTOR 0xE ,ERROR_CODE
VECTOR 0xF ,ZERO
VECTOR 0x10 ,ZERO
VECTOR 0x11 ,ERROR_CODE
VECTOR 0x12 ,ZERO
VECTOR 0x13 ,ZERO
VECTOR 0x14 ,ZERO
VECTOR 0x15 ,ZERO
VECTOR 0x16 ,ZERO
VECTOR 0x17 ,ZERO
VECTOR 0x18 ,ZERO
VECTOR 0x19 ,ZERO
VECTOR 0x1A ,ZERO
VECTOR 0x1B ,ZERO
VECTOR 0x1C ,ZERO
VECTOR 0x1D ,ZERO
VECTOR 0x1E ,ERROR_CODE
VECTOR 0x1F ,ZERO
VECTOR 0x20 ,ZERO