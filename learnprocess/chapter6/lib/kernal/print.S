TI_GDT equ 0
RPL0   equ 0
SELECTOR_VIDEO equ (0x0003<<3)+TI_GDT+RPL0

section .data
put_int_buffer dq 0        ;定义8字节缓冲区用于数字到字符的转换

[bits 32]
section .text
   ;-------------------- put_char ---------------------
   ;功能描述：把栈中的1个字符写入光标所在处
   ;---------------------------------------------------
global put_char:
put_char:
   pushad                  ;备份32位寄存器环境，需要保证gs为正确的视频段选择子，保>险起见，每次打印时都为gs赋值
   mov ax,SELECTOR_VIDEO
   mov gs,ax
;首先他是实际上是向 VGA 控制器发送命令，0x03d4是索引端口，向他里面输入操作(0x0e或0x0f),然后他将操作后的数据放入0x03d5数据端口，然后从这个端口读数据就成
;用0x0e来获取行号，0x0f来获取列号,现在这样他们都存在了ax中
   ;获取当前光标位置
   ;先获取高8位（行号)
   mov dx,0x03d4
   mov al,0x0e
   out dx,al
   mov dx,0x03d5
   in al,dx
   mov ah,al
   ;再获取低8位(列号)
   mov dx,0x03d4
   mov al,0x0f
   out dx,al
   mov dx,0x03d5
   in al,dx

   ;将光标存入bx
   mov bx,ax
   mov ecx,[esp+36]        ;pushad压入4*8=32B，加上主调函数4B的返回地址，故esp+36B
;比较时不是回车符,如果是，就进行跳转,他是将光标移动到首行
   cmp cl,0xd              ;CR是0x0d，LF是0x0a
   jz .is_carriage_return
;比较时不是\n,如果是，就进行跳转，他时将光标移动到下一行
   cmp cl,0xa
   jz .is_line_feed
;比较时不是退格符,如果是，就进行跳转，他是将光标向前移动一位
   cmp cl,0x8              ;BS(backspace)的ASCII码是8
   jz .is_backspace
;jz 和 jmp又忘了那就在看，jz(jump if zero)如果ZF为1，则跳转
;jmp(unconditional jump)无条件跳转
;好好好，那你说说为啥不用call嘻嘻
   jmp .put_other

.is_backspace:
;将字符索引左移一位
   dec bx   
;shl shift left 左移,虽然也是左移一位，但是他代表的是给他*2,一个字符占了两个字节，一个码一个属性，*2为他的显存字节偏移量
   shl bx,1                ;光标左移1位等于乘2，表示光标对应显存中的偏移字节
   mov byte [gs:bx],0x20   ;将待删除的字节补为0或空格皆可，这里的0x02只占了一个字节，代表空格
;将显存位置右移一个字节
   inc bx
   mov byte [gs:bx],0x07;这个是他的属性，设置成了白色
;将显存字节偏移量还原为字符索引
   shr bx,1
   jmp .set_cursor

.put_other:
   shl bx,1                ;光标位置用2字节表示，将光标值*2表示对应显存中的偏移字节
   mov [gs:bx],cl          ;ASCII字符本身
   inc bx
   mov byte [gs:bx],0x07   ;字符属性
   shr bx,1                ;恢复老光标值
   inc bx                  ;下一个光标值
   cmp bx,2000
;jump if less,如果小于就进行跳转
   jl .set_cursor          ;若光标值小于2000(80*25)字符，表示未写到显存的最后，则去设置新的光>标值，若超出屏幕字符大小（2000），则换行处理

;其实我就很好奇为啥上面的都跳转了更新光标寄存器，但是下面这个没有，因为他们后面还是要put_char，他会进行最终的更新，？？那为啥is_backspace他高贵？？
.is_line_feed:             ;换行符LF(\n)
.is_carriage_return:       ;回车符CR(\r)
   ;如果是CR(\r)，只要把光标移到行首就行了
   xor dx,dx               ;dx是被除数的高16位，清0
   mov ax,bx               ;ax是被除数的低16位
;这里表示每一行有80个字符
   mov si,80
;它将 ax 除以 si 中的值，并将结果存储到[低16位]ax(行数)和[高16位]dx(列数) 寄存器中。
   div si
;bx-=dx
   sub bx,dx               ;光标值取整

.is_carriage_return_end:
   add bx,80
   cmp bx,2000
.is_line_feed_end:         ;若是LF(\n)，将光标移+80即可
   jl .set_cursor

;感觉这个没用上，这个是不是应该加到2000判断那里，74行
   ;屏幕行范围是0~24，滚屏的原理是将屏幕第1~24行搬运到第0~23行，再将第23行用空格填>充
.roll_screen:              ;若超出屏幕大小，滚屏
;方向标志 DF 清除为 0,字符串操作从低向高进行
   cld
   mov ecx,960             ;2000-80=1920个字符，共1920*2=3840B，一次搬4B，共3840/4=960次
;0xc0000000他是虚拟内存开始的内核空间
   mov esi,0xc00b80a0      ;第1行行首
   mov edi,0xc00b8000      ;第0行行首
;字符串操作指令，会将esi地址指向的数据块复制到edi地址指向的数据块中，复制长度由ecx指定，ecx每次复制4B
   rep movsd

   ;将最后一行填充为空白
   mov ebx,3840
   mov ecx,80
.cls:
   mov word [gs:ebx],0x0720;0x0720是黑底白字的空格键，两个字节
   add ebx,2
   loop .cls
   mov bx,1920             ;将光标值重置为1920，最后一行首字符
;这不是和上面那个获取光标位置一样吗，对了，不是获取，是写入
;咱就是说，感觉很奇怪，为啥光标变了寄存器自己不能跟着变吗
.set_cursor:
   ;将光标设为bx值
   ;1.先设置高8位
   mov dx,0x03d4           ;索引寄存器
   mov al,0x0e             ;光标位置高8位
   out dx,al
   mov dx,0x03d5           ;通过读写数据端口0x3d5来获得或设置光标位置
   mov al,bh
   out dx,al

   ;2.再设置低8位
   mov dx,0x03d4
   mov al,0x0f
   out dx,al
   mov dx,0x03d5
   mov al,bl
   out dx,al
.put_char_done:
   popad
   ret
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;-- 功能描述：通过put_char来打印以0字符结尾的字符串 --
global put_str:
put_str:
   push ebx
   push ecx
   xor ecx,ecx
;为啥加12就是字符串的起始地址
   mov ebx,[esp+12]
.put_char_loop:
   mov cl,[ebx]
   cmp cl,0
   jz .str_over
;将起始字符串的字母赋给cx,然后将他压入栈中，给put_char函数利用，利用完之后在从栈中去除
   push ecx
   call put_char
   add esp,4
   inc ebx
   jmp .put_char_loop
.str_over:
   pop ecx
   pop ebx
   ret
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;下面函数将整数值以十六进制（16-based）格式打印的功能
   global put_int
put_int:
   pushad
   mov ebp,esp
;这里eax存放的就是要转化的二进制
   mov eax,[esp+36]        ;32B+4B返回地址
   mov edx,eax
;32位整数的16进制最多需要8字符(4*8),从缓冲区的第7个位置(第八个字符)从右向左填充，这是从低位到高位
   mov edi,7               ;put_int_buffer偏移量
   mov ecx,8               ;循环八次
   mov ebx,put_int_buffer

.16based_4bits:
;提取edx的低四位
   and edx,0x0000000F    
   cmp edx,9
;如果edx大于9，则表示是A~F的ASCII码
   jg  .is_A2F             ;A～F的ASCII码
   add edx,'0'             ;0~9的ASCII码
   jmp .store
;下面没有jmp .store的原因是按顺序执行也改他了，但是上面有是为了跳过.is_A2F
.is_A2F:
;因为10-15要表示A-F,所以减去10等于A～F的字符序+'A'得ASCII码
   sub edx,10
   add edx,'A'             ;减去10等于A～F的字符序+'A'得ASCII码
.store:
;将当前转换后的十六进制字符存入缓冲区,dl寄存器中保存了当前处理的数字对应的ASCII码字符而ebx指向字符缓冲区put_int_buffer 的起始位置
   mov [ebx+edi],dl        ;此时dl中是数字对应字符的ASCII码
   dec edi
;右移4位，因为已经处理了
   shr eax,4
   mov edx,eax
   loop .16based_4bits

;这块我真的看的不是很懂
.ready_to_print:
;他进行比较退出的时候，edi已经指向从右向左第9个字符了
   inc edi                 ;edi减退为-1
.skip_prefix_0:            ;跳过前导0
   cmp edi,8               ;edi偏移量为8的时候表示到了第9个字符
   je .full0               ;前导0有8个，说明全是0

.go_on_skip:
   mov cl,[put_int_buffer+edi]
   inc edi                 ;下一个字符
   cmp cl,'0'
   je .skip_prefix_0       ;判断下一个字符是否为'\0'
   dec edi                 ;不是'\0'，edi减1恢复当前字符
   jmp .put_each_num

.full0:
   mov cl,'0'              ;全为0，输出一个0即可
.put_each_num:
   push ecx
   call put_char
   add esp,4
   inc edi                 ;使edi指向下一个字符
   mov cl,[put_int_buffer+edi]
   cmp edi,8
   jl .put_each_num
   popad
   ret