//jmp间接绝对金转移
section call_test vstart=0x900
mov word [addr],start
jmp near [addr]
times 128 db 0
addr dw 0
start:
   mov ax,0x1234
   jmp $