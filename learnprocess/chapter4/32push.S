   %include "boot.inc"
   section push32_test vstart=0x900
   jmp loader_start
;构建gdt及其内部的描述符
   GDT_BASE:   dd    0x00000000 
	       dd    0x00000000

   CODE_DESC:  dd    0x0000FFFF 
	       dd    DESC_CODE_HIGH4

   DATA_STACK_DESC:  dd    0x0000FFFF
		     dd    DESC_DATA_HIGH4

   VIDEO_DESC: dd    0x80000007	       ; limit=(0xbffff-0xb8000)/4k=0x7
	       dd    DESC_VIDEO_HIGH4  ; 此时dpl为0

   GDT_SIZE   equ   $ - GDT_BASE
   GDT_LIMIT   equ   GDT_SIZE -	1 
   times 60 dq 0					 ; 此处预留60个描述符的空位(slot)
   SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 ; 同上
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	 ; 同上 

   gdt_ptr: dw GDT_LIMIT
            dd gdt_addr


    loader_start:
    ;---------------    准备进入保护模式     ----------------
    ;1 打开 A20
    ;2 加载 gdt
    ;3 设置 cr0 第0位为1
    ;--------------------------------------------------------
    ;打开A20
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ;加载gdt
    lgdt [gdt_ptr]

    ;设置cr0 第0位为1
    mov eax, cr0
    or eax,0x00000001
    mov cr0, eax

   ; 刷新流水线，避免分支预测的影响，这种 CPU 优化策略，最怕 jmp 跳转，
   ; 这将导致之前做的预测失效，从而起到了刷新的作用
   jmp SELECTOR_CODE:p_mode_start
[bits 32]
p_mode_start:
        mov ax, SELECTOR_DATA
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov esp,0x900
        push byte 0x7
        push word 0x8
        push dword 0x9
        jmp $
        