section call_test vstart=0x900
jmp short start;会报错，可删除short更改为长调转来避免范围问题或者将他改为near
times 128 db 0
start:
    mov ax,0x1234
    jmp $