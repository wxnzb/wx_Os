# 定义变量
CC = gcc
ASM = nasm
LD = ld
DD = dd

CFLAGS = -I lib/kernal/ -I lib/ -I kernal/ -c -fno-builtin
ASMFLAGS = -f elf
LDFLAGS = -Ttext 0xc0001500 -e main -o build/kernal.bin
TARGET = build/kernal.bin

# 文件列表
C_SRCS = kernal/main.c kernal/interrupt.c kernal/init.c
ASM_SRCS = lib/kernal/print.S kernal/kernal.S
OBJS = $(C_SRCS:.c=.o) $(ASM_SRCS:.S=.o)

# 默认目标
all: build $(TARGET)

# 创建 build 目录
build:
	mkdir -p build

# 编译 C 文件
%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# 编译汇编文件
%.o: %.S
	$(ASM) $(ASMFLAGS) -o $@ $<

# 链接目标文件
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS)

# 将内核映像写入硬盘映像
install: $(TARGET)
	$(DD) if=$(TARGET) of=/home/work/my_workspace/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc

# 清理生成的文件
clean:
	rm -f build/*.o build/kernal.bin

.PHONY: all install clean build