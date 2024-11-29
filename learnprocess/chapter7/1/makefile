# #操作步骤
# gcc -m32 -I lib/kernal/ -m32 -I lib/ -m32 -I kernal/ -c -fno-builtin -fno-stack-protector -o build/main.o kernal/main.c
# gcc -m32 -I lib/kernal/ -m32 -I lib/ -m32 -I kernal/ -c -fno-builtin -fno-stack-protector -o build/interrupt.o kernal/interrupt.c，有1,2之分
# gcc -m32 -I lib/kernal/ -m32 -I lib/ -m32 -I kernal/ -c -fno-builtin -fno-stack-protector -o build/init.o kernal/init.c
# nasm -f elf -o build/print.o lib/kernal/print.S
# nasm -f elf -o build/kernal.o kernal/kernal.S，有1,2之分
# ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernal.bin build/main.o build/init.o build/interrupt.o build/print.o build/kernal.o
# dd if=/home/sweet/git/vc/learnprocess/chapter7/build/kernal.bin of=/home/sweet/git/vc/JY/wx.img bs=512 count=200 seek=9 conv=notrunc

#我真服了，让ai生成一个makefile文件怎么就这么难,嘻嘻，终于生成拉

#第三次多加了一个时钟频率
#gcc -m32 -I lib/kernal -c -o build/timer.o device/timer.c
# ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernal.bin build/main.o build/init.o build/interrupt.o build/print.o build/kernal.o build/timer.o


# 定义编译器和汇编器
CC := gcc
AS := nasm
LD := ld

# 定义编译选项
CFLAGS := -m32 -I lib/kernal/ -I lib/ -I kernal/ -c -fno-builtin -fno-stack-protector
ASFLAGS := -f elf
LDFLAGS := -m elf_i386 -Ttext 0xc0001500 -e main

# 定义目标文件和输出文件
OBJS := build/main.o build/interrupt.o build/init.o build/print.o build/kernal.o
KERNEL_BIN := build/kernal.bin
IMG_FILE := /home/sweet/git/vc/JY/wx.img

# 默认目标
all: $(KERNEL_BIN) $(IMG_FILE)

# 编译C文件
build/main.o: kernal/main.c
	$(CC) $(CFLAGS) -o $@ $<

build/interrupt.o: kernal/interrupt.c
	$(CC) $(CFLAGS) -o $@ $<

build/init.o: kernal/init.c
	$(CC) $(CFLAGS) -o $@ $<

# 汇编汇编文件
build/print.o: lib/kernal/print.S
	$(AS) $(ASFLAGS) -o $@ $<

build/kernal.o: kernal/kernal.S
	$(AS) $(ASFLAGS) -o $@ $<

# 链接目标文件
$(KERNEL_BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# 将二进制文件写入映像文件
$(IMG_FILE): $(KERNEL_BIN)
	dd if=$(KERNEL_BIN) of=$(IMG_FILE) bs=512 count=200 seek=9 conv=notrunc

# 清理目标
clean:
	rm -f $(OBJS) $(KERNEL_BIN)

.PHONY: all clean
