mov ax,0xb800
mov gs,ax

mov ax,0x600
;mov bx,0x700;自己尝试，感觉这局完全没用
mov cx,0
mov dx,0x181f
int 0x10
mov byte [gs:0x00],'H'
mov byte [gs:0x01],0x0a
mov byte [gs:0x02],'A'
mov byte [gs:0x03],0x0a
mov byte [gs:0x04],'P'
mov byte [gs:0x05],0x0a

mov byte [gs:0x06],'P'
mov byte [gs:0x07],0xaa
mov byte [gs:0x08],'Y'
mov byte [gs:0x09],0xaa
mov byte [gs:0x0a],'!'
mov byte [gs:0x0b],0x5a

