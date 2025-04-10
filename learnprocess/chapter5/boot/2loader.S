%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp loader_start
    ;-------------------------------------------------                                                    
GDT_BASE:                                               ; 构建gdt及其内部的描述符
    dd 0x00000000 
	dd 0x00000000

CODE_DESC:  
    dd 0x0000FFFF 
	dd DESC_CODE_HIGH4

DATA_STACK_DESC:  
    dd 0x0000FFFF
    dd DESC_DATA_HIGH4

VIDEO_DESC: 
    dd 0x80000007	                                    ; limit=(0xbffff-0xb8000)/4k=0x7
    dd DESC_VIDEO_HIGH4                                 ; 此时dpl已改为0

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1 
    times 60 dq 0
    ;times 59 dq 0
    ;times 5 db 0					                    ; 此处预留60个描述符的空间
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0       ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	    ; 同上
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	    ; 同上 

    total_mem_bytes dd 0				                ; total_mem_bytes用于保存内存容量,以字节为单位,此位置比较好记。
                                                        ; 当前偏移loader.bin文件头0x200字节,loader.bin的加载地址是0x900,
                                                        ; 故total_mem_bytes内存中的地址是0xb00.将来在内核中咱们会引用此地址	 
                                                        
    gdt_ptr dw GDT_LIMIT                                ; 定义加载进入GDTR的数据，前2字节是gdt界限，后4字节是gdt起始地址，
	        dd  GDT_BASE

    ards_buf times 244 db 0                             ; 人工对齐:total_mem_bytes4字节+gdt_ptr6字节+ards_buf244字节+ards_nr2,共256字节
    ards_nr dw 0	
;-------------------------------------------------------
loader_start:
                                                        ; -------  int 15h eax = 0000E820h ,edx = 534D4150h ('SMAP') 获取内存布局  -------

    xor ebx, ebx		                                ; 第一次调用时，ebx值要为0
    mov edx, 0x534d4150	                                ; edx只赋值一次，循环体中不会改变
    mov di, ards_buf	                                ; ards结构缓冲区
.e820_mem_get_loop:	                                    ; 循环获取每个ARDS内存范围描述结构
    mov eax, 0x0000e820	                                ; 执行int 0x15后,eax值变为0x534d4150,所以每次执行int前都要更新为子功能号。
    mov ecx, 20		                                    ; ARDS地址范围描述符结构大小是20字节
    int 0x15
    add di, cx		                                    ; 使di增加20字节指向缓冲区中新的ARDS结构位置
    inc word [ards_nr]	                                ; 记录ARDS数量
    cmp ebx, 0		                                    ; 若ebx为0且cf不为1,这说明ards全部返回，当前已是最后一个
    jnz .e820_mem_get_loop

                                                        ; 在所有ards结构中，找出(base_add_low + length_low)的最大值，即内存的容量。
    mov cx, [ards_nr]	                                ; 遍历每一个ARDS结构体,循环次数是ARDS的数量
    mov ebx, ards_buf 
    xor edx, edx		                                ; edx为最大的内存容量,在此先清0
.find_max_mem_area:	                                    ; 无须判断type是否为1,最大的内存块一定是可被使用
    mov eax, [ebx]	                                    ; base_add_low
    add eax, [ebx+8]	                                ; length_low
    add ebx, 20		                                    ; 指向缓冲区中下一个ARDS结构
    cmp edx, eax		                                ; 冒泡排序，找出最大,edx寄存器始终是最大的内存容量
    jge .next_ards 
    mov edx, eax                                        ; edx 为总内存大小
.next_ards:
    loop .find_max_mem_area 
    jmp .mem_get_ok 

                                                        ; ------ int 15h ax = E801h 获取内存大小，最大支持 4G ------ 
                                                        ; 返回后, ax cx 值一样,以 1KB 为单位，bx dx 值一样，以 64KB 为单位
                                                        ; 在 ax 和 cx 寄存器中为低 16MB，在 bx 和 dx 寄存器中为 16MB 到 4GB 
.e820_failed_so_try_e801:
    mov ax,0xe801 
    int 0x15 
    jc .e801_failed_so_try88                            ; 若当前 e801 方法失败，就尝试 0x88 方法

                                                        ; 1 先算出低 15MB 的内存
                                                        ; ax 和 cx 中是以 1KB 为单位的内存数量，将其转换为以 byte 为单位
    mov cx, 0x400                                       ; 将值 0x400 (1024) 存入 CX 寄存器
    mul cx                                              ; 用 AX 寄存器中的值乘以 CX 的值，结果存入 DX:AX
    shl edx, 16                                         ; 将 EDX 中的值左移 16 位，结果存入 EDX
    and eax, 0x0000FFFF                                 ; 仅保留 EAX 的低 16 位，高 16 位清零
    or edx, eax                                         ; 将 EAX 的值合并到 EDX 的低 16 位
    add edx, 0x100000                                   ; 向 EDX 中的值加上 0x100000 (1MB)
    mov esi, edx                                        ; 将 EDX 的值存入 ESI 寄存器
                                                        ; 先把低 15MB 的内存容量存入 esi 寄存器备份

                                                        ; 2 再将 16MB 以上的内存转换为 byte 为单位
                                                        ; 寄存器 bx 和 dx 中是以 64KB 为单位的内存数量
    xor eax,eax 
    mov ax,bx 
    mov ecx, 0x10000                                    ; 0x10000 十进制为 64KB 
    mul ecx                                             ; 32 位乘法，默认的被乘数是 eax，积为 64 位
                                                        ; 高 32 位存入 edx，低 32 位存入 eax 
    add esi,eax 
                                                        ; 由于此方法只能测出 4GB 以内的内存，故 32 位 eax 足够了
                                                        ; edx 肯定为 0，只加 eax 便可
    mov edx,esi                                         ; edx 为总内存大小
    jmp .mem_get_ok 

                                                        ; ----- int 15h     ah = 0x88 获取内存大小，只能获取 64MB 之内 ----- 
.e801_failed_so_try88: 
    mov ah, 0x88                                        ; 设置 AH 寄存器为 0x88，准备调用 BIOS 中断
    int 0x15                                            ; 调用 BIOS 中断 0x15，子功能 0x88，获取内存信息
    jc .error_hlt                                       ; 如果调用失败（进位标志被设置），跳转到 .error_hlt 标签

    and eax, 0x0000FFFF                                 ; 清除 EAX 的高 16 位，仅保留低 16 位
                                                        ; AX 存储的是以 1KB 为单位的内存容量
    
                                                        ; 16 位乘法，被乘数是 AX，积为 32 位。积的高 16 位在 DX 中，积的低 16 位在 AX 中
    mov cx, 0x400                                       ; 将值 0x400 (1024) 存入 CX 寄存器
                                                        ; 用于将 AX 中的内存容量转换为字节
    
    mul cx                                              ; 进行乘法运算，AX * CX，结果为 32 位，高 16 位在 DX 中，低 16 位在 AX 中
    shl edx, 16                                         ; 将 DX 的值左移 16 位，结果存储在 EDX 中
    or edx, eax                                         ; 将 EAX 中的低 16 位值与 EDX 进行按位或操作，合并结果存储在 EDX 中
    add edx, 0x100000                                   ; 向 EDX 中的值加上 0x100000 (1MB)
                                                        ; 因为 0x88 子功能只会返回 1MB 以上的内存，所以要加上 1MB


.mem_get_ok:
mov [total_mem_bytes], edx                              ; 将内存换为 byte 单位后存入 total_mem_bytes 处
;--------------------------------------------------------

                                                        ; -----------------   准备进入保护模式   ------------------------------------------
                                                        ; 1 打开A20
                                                        ; 2 加载gdt
                                                        ; 3 将cr0的pe位置1


                                                        ; -----------------  打开A20  ----------------
    in al, 0x92
    or al, 0000_0010B
    out 0x92,al

                                                        ; -----------------  加载GDT  ----------------
    lgdt [gdt_ptr]


                                                        ; -----------------  cr0第0位置1  ----------------
    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax

                                                        ; jmp dword SELECTOR_CODE:p_mode_start	    
    jmp  SELECTOR_CODE:p_mode_start	                    ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					                                    ; 这将导致之前做的预测失效，从而起到了刷新的作用。

                                                        
    .error_hlt:		                                    ; 出错则挂起
    hlt
;------------------------------------------------------
[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    ;----------------------------------------------------

    call setup_page                                     ; 创建页目录表的函数,我们的页目录表必须放在1M开始的位置，所以必须在开启保护模式后运行
                                                    ; 要将描述符表地址及偏移量写入内存 gdt_ptr，一会儿用新地址重新加载
;这块sgdt会自动去GDTR(全局描述符表寄存器)中读取gdt的大小（2）这也就是为啥要加2(为了跳过地址获取gdt的内存地址)和基址
;因为开启了分页模式，这块都要把他加上0xc0000000，因为内核在3G以上，所以这块的地址也要加上0xc0000000
    sgdt [gdt_ptr]                                      ; 存储到原来 gdt 所有的位置
                                                        ; 以下两句是将gdt描述符中视频段描述符中的段基址+0xc0000000
    mov ebx, [gdt_ptr + 2]                              ; ebx中存着GDT_BASE
;+4表示的是字节偏移而不是位偏移，这是因为汇编默认使用的就是以字节为单位进行偏移的
;为啥不和上面一样用add而用or因为段描述符高4字节不知有段基址，还有其他属性，所以用or操作，只修改段基址
    or dword [ebx + 0x18 + 4], 0xc0000000               ; 视频段是第3个段描述符,每个描述符是8字节,故0x18 = 24，然后+4，是取出了视频段段描述符的高4字节。然后or操作，段基址最高位+c
                                            
    add dword [gdt_ptr + 2], 0xc0000000                 ; 将gdt的基址加上0xc0000000使其成为内核所在的高地址

    add esp, 0xc0000000                                 ; 将栈指针同样映射到内核地址

    mov eax, PAGE_DIR_TABLE_POS                         ; 把页目录地址赋给cr3
    mov cr3, eax
                                                        
    mov eax, cr0                                        ; 打开cr0的pg位(第31位)
    or eax, 0x80000000  
    mov cr0, eax
                                                        
    lgdt [gdt_ptr]                                      ; 在开启分页后,用gdt新的地址重新加载
;为啥这样就可以的打印出来，不明白原理？？？？
    mov byte [gs:160], 'V'                              ; 视频段段基址已经被更新,用字符v表示virtual addr

    jmp $
    ;---------------------------------------------------;建立页表！！！！
;每一个页目录项有1024个页表每一个页表占4字节通过这个可以找到相应的页表，每一个页表有1024项，每一项又占了4字节，所以这个是二级页表                                                  
setup_page:                                             ; ------------------------------------------   创建页目录及页表  -------------------------------------
                                                        ; ----------------以下6行是将1M开始的4KB置为0，将页目录表初始化
    mov ecx, 4096                                       ; 创建4096个byte 0，循环4096次
    mov esi, 0                                          ; 用esi来作为偏移量寻址
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

                                                        ; ----------------初始化页目录表，让0号项与768号指向同一个页表，该页表管理从0开始4M的空间
.create_pde:				                            ; 一个页目录表项可表示4MB内存,这样0xc03fffff以下的地址和0x003fffff以下的地址都指向相同的页表，这是为将地址映射为内核地址做准备
    mov eax, PAGE_DIR_TABLE_POS                         ; eax中存着页目录表的位置
;!!!等于是在内存中页目录表和也表示挨在一起的，他直接加4kb是为了跳过页目录表到达第一个页表的起始内存地址
    add eax, 0x1000 			                        ; 在页目录表位置的基础上+4K（页目录表的大小），现在eax中第一个页表的起始位置
    mov ebx, eax				                        ; 此处为ebx赋值，现在ebx存着第一个页表的起始位置
    or eax, PG_US_U | PG_RW_W | PG_P	                ; 页目录项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
                                                        ; 现在eax中的值符合一个页目录项的要求了，高20位是一个指向第一个页表的4K整数倍地址，低12位是相关属性设置
; 第0号条目 负责管理 0x000000 到 0x003FFFFF 这段地址空间
;第768号条目 负责管理 0xC0000000 到 0xC03FFFFF 这段地址空间
;那为啥要让用户空间和内和空间共享一个页表？？？？？？
    mov [PAGE_DIR_TABLE_POS + 0x0], eax                 ; 页目录表0号项写入第一个页表的位置(0x101000)及属性(7)
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax               ; 页目录表768号项写入第一个页表的位置(0x101000)及属性(7)
					                                    
    sub eax, 0x1000                                     ; ----------------- 使最后一个目录项指向页目录表自己的地址，为的是将来动态操作页表做准备
;页目录项的地址赋给了页目录项中的最后一个页表1024*4-4
;目的：为自我引用或循环引用机制做准备  
    mov [PAGE_DIR_TABLE_POS + 4092], eax	            ; 属性包含PG_US_U是为了将来init进程（运行在用户空间）访问这个页目录表项
                                                        
    mov ecx, 256				                        ; -----------------初始化第一个页表，因为我们的操作系统不会超过1M，所以只用初始化256项
    mov esi, 0                                          ; esi来做寻址页表项的偏移量
    mov edx, PG_US_U | PG_RW_W | PG_P	                ; 属性为7,US=1,RW=1,P=1
.create_pte:				                            ; 创建Page Table Entry
    mov [ebx+esi*4],edx			                        ; 此时的ebx已经在上面通过eax赋值为0x101000,也就是第一个页表的地址 
    add edx,4096                                        ; edx指向下一个4kb空间，且已经设定好了属性，故edx中是一个完整指向下一个4kb物理空间的页表表项
    inc esi                                             ; 寻址页表项的偏移量+1
    loop .create_pte                                    ; 循环设定第一个页表的256项

                                                        ; -------------------初始化页目录表769号-1022号项，769号项指向第二个页表的地址（此页表紧挨着上面的第一个页表），770号指向第三个，以此类推
    mov eax, PAGE_DIR_TABLE_POS                         ; eax存页目录表的起始位置
    add eax, 0x2000 		                            ; 此时eax为第二个页表的位置
    or eax, PG_US_U | PG_RW_W | PG_P                    ; 设置页目录表项相关属性，US,RW和P位都为1，现在eax中的值是一个完整的指向第二个页表的页目录表项
    mov ebx, PAGE_DIR_TABLE_POS                         ; ebx现在存着页目录表的起始位置
    mov ecx, 254			                            ; 要设置254个表项
;??为啥是769阿，真奇怪
    mov esi, 769                                        ; 要设置的页目录表项的偏移起始
.create_kernel_pde:
    mov [ebx+esi*4], eax                                ; 设置页目录表项
    inc esi                                             ; 增加要设置的页目录表项的偏移
    add eax, 0x1000                                     ; eax指向下一个页表的位置，由于之前设定了属性，所以eax是一个完整的指向下一个页表的页目录表项
    loop .create_kernel_pde                             ; 循环设定254个页目录表项
    ret
    ;----------------------------------------------------