%include "boot.inc"
SECTION LOADER vstart=LOADER_BASE_ADDR     ;同书上，设置为0x900
LOADER_STACK_TOP equ LOADER_BASE_ADDR      ;初始化栈顶，0x900向下为栈空间
jmp loader_start
;??首先第一个疑问就是上面三个选择子他们的第一个表示的都是段的大小，而最后一个既有段基址也有段大小，这表示方式不一样计算机咋识别呀
;??DESC_G_4K我看他们dd都有这个，那么证明他们都是以页为单位，那为啥上面那三个没有/4k,还是以字节为单位，真的好奇怪
   ;构建GDT及其内部的描述符
   GDT_BASE:   dd   0x00000000             ;没用的第0个段描述符
               dd   0x00000000
   CODE_DESC:  dd   0x0000FFFF
               dd   DESC_CODE_HIGH4
   DATA_STACK_DESE:   dd   0x0000FFFF
                      dd   DESC_DATA_HIGH4
;，这里是因为用户的内存是从0x00000000-0xbfffffff,内核0xc0000000-0xffffffff    
;那我还是很好奇，那为啥他的段基址不是从0xb800开始的而是从0x80开始的              
   VIDEO_DESC: dd   0x80000007                ;limit=(0xbffff-0xb8000)/4k=0x7
               dd   DESC_VIDEO_HIGH4          ;此时DPL为0
   GDT_SIZE    equ  $-GDT_BASE                ;地址差作尺寸：当前行地址-GDT_BASE地址
   GDT_LIMIT   equ  GDT_SIZE-1

   times 59 dq 0                              ;dq 定义4字/8字节
   times 5 db 0

   total_mem_bytes dd 0;嘿嘿*>*,他的地址在0xb00,0x900+0x200,0x200=512字节=(4[4个段描述符]+60[预留的60个空间])*8字节
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   SELECTOR_CODE   equ   (0x0001<<3)+TI_GDT+RPL0
      ;相当于(CODE_DESC-GDT_BASE)/8+TI_GDT+RPL0
   SELECTOR_DATA   equ   (0x0002<<3)+TI_GDT+RPL0
   SELECTOR_VIDEO  equ   (0x0003<<3)+TI_GDT+RPL0

   ;以下是GDT的指针GDTR,6B/48bit，前2字节是GDT界限，后4字节是GDT起始地址
   gdt_ptr     dw  GDT_LIMIT;define word占了两个2字节
               dd  GDT_BASE;define double word占4字节

   ards_buf times 244 db 0;244字节//每个ARDS结构体20字节，最多12个那也应该是240,这咋还多处4字节，作者呢，纯属是为了对其，还是浅浅地优点强迫症
   ;total_mem_bytes是 4 字节，gdt_ptr 是 6 字节，ards_buf 是 244 字节，ards_nr 是 2 字节，加起来的和是 256 字节，即 0x100。加
   ;+上 total_mem_bytes 在文件内偏移地址是 0x200，所以 loader_start 在文件内的偏移地址是 0x100+0x200=0x300
   ards_nr dw 0;2个字节

loader_start:

   ;int 15h eax=0000E820h, edx=534D4150h('SMAP'的ASCII码)获取内存布局
   xor ebx,ebx                 ;第一次调用时，ebx要为0
   mov edx,0x534d4150          ;edx只赋值一次，循环体中不会改变，用于签名校验，将 edx 设置为 "SMAP" 的 ASCII 码（十六进制表示）
   mov di,ards_buf             ;ards结构缓冲区
.e820_mem_get_loop:            ;循环获取每个ARDS内存范围描述结构
   mov eax,0x0000e820          ;执行int 0x15后，eax值变为0x534d4150，所以每次执行int前都要更新为子>功能号
   mov ecx,20                  ;ARDS地址范围描述符结构大小是20字节
   int 0x15                    ;0x15为获取内存容量的中断号
   jc .e820_failed_so_try_e801 ;若cf位为1则有错误发生，尝试0xe801子功能
   add di,cx                   ;使di增加20字节指向缓冲区中新的ARDS结构位置
   inc word [ards_nr]          ;记录ARDS数量
   cmp ebx,0                   ;若ebx为0且cf不为1，这说明ards全部返回，当前已是最后一个

   jnz .e820_mem_get_loop

   ;在所有ards结构中找出(base_add_low+length_low)的最大值，即内存的容量
   mov cx,[ards_nr]            ;遍历每个ARDS结构体，循环次数是ARDS的数量
   mov ebx,ards_buf
   xor edx,edx                 ;此后edx用于记录最大内存容量，在此先请0
.find_max_mem_area:            ;无需判断type是否为1，最大的内存块一定是可以被使用的
   mov eax,[ebx]               ;base_add_low
   add eax,[ebx+8]             ;length_low
   add ebx,20                  ;指向缓冲区下一个ARDS结构
   cmp edx,eax                 ;冒泡排序，找出最大，edx寄存器始终是最大的内存容量
   jge .next_ards
   mov edx,eax                 ;edx为总内存大小
.next_ards:
   loop .find_max_mem_area
   jmp .mem_get_ok

   ;----- int 15h ax=E801h 获取内存大小，最大支持4G -----
   ;返回后，ax cx值一样，以KB为单位，bx dx值一样，以64KB为单位
   ;在ax和cx寄存器中为低16MB，在bx和dx寄存器中为16MB到4GB
.e820_failed_so_try_e801:
   mov ax,0xe801
   int 0x15
   jc .e801_failed_so_try88    ;若当前e801方法也失败，则尝试0x88方法

   ;1.先算出低15MB内存。ax cx是以KB为单位的内存数量，将其转换为以Byte为单位
   mov cx,0x400                ;0x400十进制为1K。cx和ax一样，cx用作乘数
   mul cx
   shl edx,16
   and eax,0x0000FFFF
   or edx,eax
   add edx,0x100000            ;ax只是15MB，故要追加1MB
   mov esi,edx                 ;先把低15MB内存容量存入esi寄存器备份

   ;2.再将16MB以上的内存转换为Byte为单位。bx dx是以64KB为单位的内存数量
   xor eax,eax
   mov ax,bx
   mov ecx,0x10000             ;0x10000十进制为64KB
   mul ecx                     ;32位乘法，默认的被乘数是eax，积为64位。高32位存入edx，低32位存入eax
   add esi,eax                 ;4GB内存，edx为0，低32位足矣
   mov edx,esi                 ;edx为总内存大小
   jmp .mem_get_ok

   ;----- int 15h ah=0x88获取内存大小，只能获取64MB之内 -----
.e801_failed_so_try88:
   ;int 15h后，ax存入的是以KB为单位的内存容量
   mov ah,0x88
   int 0x15
   jc .error_hlt
   and eax,0x0000FFFF

   ;16位乘法，被乘数是ax，积为32位。积的高16位在dx中，低16位在ax中
;我不理解就是它保留了eax的第16位，然后乘以0x400，也就是2的10次方，那也就2的26次方呀，根本就用不了2的32次方
;他是无符号相乘，超过32位会保存在edx中，首先我感觉不会超过，其次超过的话他左翼16不害怕数据丢失吗？？？？？

   mov cx,0x400
   mul cx
   shl edx,16                  ;把edx移动高16位
   or edx,eax                  ;把低16位组合到edx，即为32为积
   add edx,0x100000            ;0x88只会返回1MB以上的内存，故实际内存还要加上1MB

.error_hlt:
   jmp $

.mem_get_ok:
   mov [total_mem_bytes],edx   ;将内存换位Byte单位后存入total_mem_bytes处

   ;--------------------- 准备进入保护模式 ------------------------
   ;1 打开A20
   ;2 加载GDT
   ;3 将cr0的PE位置1

   ;--------------------------- 打开A20 ---------------------------
   in al,0x92
   or al,0000_0010B            ;简单说，将端口0x92的第1位置1即可
   out 0x92,al

   ;--------------------------- 加载GDT ---------------------------
   lgdt [gdt_ptr]              ;load GDT [addr]

   ;-------------------------- cr0第0位置1 ------------------------
   mov eax,cr0
   or eax,0x00000001
   mov cr0,eax

   jmp dword SELECTOR_CODE:p_mode_start     ;刷新流水线。因为要远转移，cs更新，所>以流水线上的其它指令都没用了，就会刷新

[bits 32]     ;开启32位指令
p_mode_start:
   mov ax,SELECTOR_DATA
   mov ds,ax
   mov es,ax
   mov ss,ax
   mov esp,LOADER_STACK_TOP
   mov ax,SELECTOR_VIDEO
;这里把段选择子赋给寄存器，计算机执行的步骤是这样的
;这样做了之后，cpu会识别选子的索引，然后去GDT中找到他的基址啥的相关信息
   mov gs,ax

   mov byte [gs:160],'P'       ;第2行首字符打印P

   jmp $