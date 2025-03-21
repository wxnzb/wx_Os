    %include "boot.inc"
    section loader vstart=LOADER_BASE_ADDR
    LOADER_STACK_TOP equ LOADER_BASE_ADDR
    jmp loader_start					                ;loader一进来是一大堆GDT段描述符数据，无法执行，所以要跳过
   
                                                        
GDT_BASE:                                               ;构建gdt及其内部的描述符
    dd 0x00000000 
	dd 0x00000000

CODE_DESC:  
    dd 0x0000FFFF 
	dd DESC_CODE_HIGH4

DATA_STACK_DESC:  
    dd 0x0000FFFF
    dd DESC_DATA_HIGH4

VIDEO_DESC: 
;这个段描述符地32位的高4字节为段基址，不应该为0xb8000吗？为什么是0x0800？
;因为他的高32为的0—7你可以自己去看看，嘻嘻
    dd 0x80000007	                                    ;limit=(0xbffff-0xb8000)/4k=0x7
    dd DESC_VIDEO_HIGH4                                 ; 此时dpl已改为0

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1 
    times 60 dq 0					                    ; 此处预留60个描述符的空间
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0       ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	    ; 同上
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	    ; 同上 
    
gdt_ptr dw GDT_LIMIT                                    ;定义加载进入GDTR的数据，前2字节是gdt界限，后4字节是gdt起始地址，
	    dd  GDT_BASE
; 很好其为啥这句话打印在了最下面没有覆盖1 mbr或者其他～～～～～////？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
loadermsg db '2 loader in real.'

loader_start:

                                                        ;------------------------------------------------------------
                                                        ;INT 0x10    功能号:0x13    功能描述:打印字符串
                                                        ;------------------------------------------------------------
                                                        ;输入:
                                                        ;AH 子功能号=13H
                                                        ;BH = 页码
                                                        ;BL = 属性(若AL=00H或01H)
                                                        ;CX＝字符串长度
                                                        ;(DH、DL)＝坐标(行、列)
                                                        ;ES:BP＝字符串地址 
                                                        ;AL＝显示输出方式
                                                        ;   0——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置不变
                                                        ;   1——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置改变
                                                        ;   2——字符串中含显示字符和显示属性。显示后，光标位置不变
                                                        ;   3——字符串中含显示字符和显示属性。显示后，光标位置改变
                                                        ;无返回值
    mov sp,LOADER_BASE_ADDR
    mov	bp,loadermsg                                    ; ES:BP = 字符串地址
    mov	cx,17			                                ; CX = 字符串长度
    mov	ax,0x1301		                                ; AH = 13,  AL = 01h
    mov	bx,0x001f		                                ; 页号为0(BH = 0) 蓝底粉红字(BL = 1fh)
    mov	dx,0x1800		                                ;行列号
    int	0x10                                            ; 10h 号中断

                                                        ;-----------------   准备进入保护模式   ------------------------------------------
                                                        ;1 打开A20
                                                        ;2 加载gdt
                                                        ;3 将cr0的pe位置1


                                                        ;-----------------  打开A20  ----------------
    in al, 0x92
    or al, 0000_0010B
    out 0x92,al

                                                        ;-----------------  加载GDT  ----------------
    lgdt [gdt_ptr]


                                                        ;-----------------  cr0第0位置1  ----------------
    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax

                                                        ;jmp dword SELECTOR_CODE:p_mode_start	    
    jmp  SELECTOR_CODE:p_mode_start	                    ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					                                    ; 这将导致之前做的预测失效，从而起到了刷新的作用。

[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    ;所以160有时便一个数他是位置跑了，有时有变成颜色了，这到底是啥？？？？？？？？？？？？？？？？？？？？？？？？？？？？
    mov byte [gs:160], 'P'
   ; mov byte [gs:161], 0x1F     ; 设置字符属性（如蓝底白字）这是自己加的
    ;为啥他刚好打印在了1 mbr下面那一行

    jmp $