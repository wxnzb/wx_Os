# BUILD_DIR = ./build
# ENTRY_POINT = 0xc0001500
# AS = nasm
# CC = gcc
# LD = ld
# LIB = -I lib/ -I lib/kernal/ -I lib/user/ -I kernal/ -I device/ -I thread/
# ASFLAGS = -f elf
# CFLAGS = -Wall -m32 -fno-stack-protector $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes
# LDFLAGS =  -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernal.map
# OBJS = $(BUILD_DIR)/main.o  $(BUILD_DIR)/init.o   $(BUILD_DIR)/interrupt.o \
# 	$(BUILD_DIR)/timer.o  $(BUILD_DIR)/kernal.o $(BUILD_DIR)/print.o \
# 	$(BUILD_DIR)/debug.o  $(BUILD_DIR)/string.o $(BUILD_DIR)/memory.o \
# 	$(BUILD_DIR)/bitmap.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o \
# 	$(BUILD_DIR)/switch.o $(BUILD_DIR)/console.o $(BUILD_DIR)/sync.o
# ##############     c代码编译     ###############
# $(BUILD_DIR)/main.o: kernal/main.c lib/kernal/print.h \
# 	lib/stdint.h kernal/init.h lib/string.h kernal/memory.h \
# 	thread/thread.h kernal/interrupt.h  device/console.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/init.o: kernal/init.c kernal/init.h lib/kernal/print.h \
# 	lib/stdint.h kernal/interrupt.h device/timer.h kernal/memory.h \
# 	thread/thread.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/interrupt.o: kernal/interrupt.c kernal/interrupt.h \
# 	lib/stdint.h kernal/global.h lib/kernal/io.h lib/kernal/print.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h\
# 	lib/kernal/io.h lib/kernal/print.h thread/thread.h kernal/interrupt.h \
# 	kernal/debug.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/debug.o: kernal/debug.c kernal/debug.h \
# 	lib/kernal/print.h lib/stdint.h kernal/interrupt.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/string.o: lib/string.c lib/string.h \
# 	kernal/debug.h kernal/global.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/memory.o: kernal/memory.c kernal/memory.h \
# 	lib/stdint.h lib/kernal/bitmap.h kernal/debug.h lib/string.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/bitmap.o: lib/kernal/bitmap.c lib/kernal/bitmap.h \
# 	lib/string.h kernal/interrupt.h lib/kernal/print.h kernal/debug.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h \
# 	lib/stdint.h lib/string.h kernal/global.h kernal/memory.h \
# 	kernal/interrupt.h kernal/debug.h lib/kernal/print.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/list.o: lib/kernal/list.c lib/kernal/list.h \
# 	kernal/interrupt.h lib/stdint.h kernal/debug.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/console.o: device/console.c device/console.h \
# 	lib/kernal/print.h  lib/stdint.h  thread/thread.h  \
# 	thread/sync.h
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/sync.o: thread/sync.c thread/sync.h lib/kernal/list.h \
# 	lib/stdint.h thread/thread.h kernal/interrupt.h  \
# 	kernal/debug.h 
# 	$(CC) $(CFLAGS) $< -o $@
# ##############    汇编代码编译    ###############
# $(BUILD_DIR)/kernal.o: kernal/kernal.S
# 	$(AS) $(ASFLAGS) $< -o $@

# $(BUILD_DIR)/print.o: lib/kernal/print.S
# 	$(AS) $(ASFLAGS) $< -o $@

# $(BUILD_DIR)/switch.o: thread/switch.S
# 	$(AS) $(ASFLAGS) $< -o $@


# ##############    链接所有目标文件    #############
# $(BUILD_DIR)/kernal.bin: $(OBJS)
# 	$(LD) $(LDFLAGS) $^ -o $@

# .PHONY : mk_dir hd clean all

# mk_dir:
# 	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi

# hd:
# 	dd if=$(BUILD_DIR)/kernal.bin \
# 	of=/home/sweet/git/vc/JY/wx.img\
# 	bs=512 count=200 seek=9 conv=notrunc

# clean:
# 	cd $(BUILD_DIR) && rm -f  ./*

# build: $(BUILD_DIR)/kernal.bin

# all: mk_dir build hd
#定义一大堆变量，实质就是将需要多次重复用到的语句定义一个变量方便使用与替换
#一定要写对路径和参数
BUILD_DIR=./build
ENTRY_POINT=0xc0001500
HD60_PATH=/home/sweet/git/vc/JY/wx.img
AS=nasm
CC=gcc
LD=ld
LIB= -I lib/ -I lib/kernal/ -I lib/user/ -I kernal/ -I device/ -I thread
ASFLAGS= -f elf
CFLAGS= -m32 $(LIB) -c -fno-builtin -fno-stack-protector -Wall -W -Wstrict-prototypes -Wmissing-prototypes
#-Wall warning wall的意思，产生尽可能多警告信息，-fno-builtin不要采用内部函数，
#-W 会显示警告，但是只显示编译器认为会出现错误的警告
#-Wstrict-prototypes 要求函数声明必须有参数类型，否则发出警告。-Wmissing-prototypes 必须要有函数声明，否则发出警告

LDFLAGS= -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernal.map -m elf_i386
#-Map,生成map文件，就是通过编译器编译之后，生成的程序、数据及IO空间信息的一种映射文件
#里面包含函数大小，入口地址等一些重要信息

OBJS=$(BUILD_DIR)/main.o $(BUILD_DIR)/init.o \
	$(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernal.o \
	$(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o \
	$(BUILD_DIR)/bitmap.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o \
	$(BUILD_DIR)/list.o $(BUILD_DIR)/switch_to.o $(BUILD_DIR)/sync.o \
	$(BUILD_DIR)/console.o
#顺序最好是调用在前，实现在后

######################编译两个启动文件的代码#####################################
boot:$(BUILD_DIR)/mbr.o $(BUILD_DIR)/loader.o
$(BUILD_DIR)/mbr.o:boot/mbr.S
	$(AS) -I boot/include/ -o build/mbr.o boot/mbr.S
	
$(BUILD_DIR)/loader.o:boot/loader.S
	$(AS) -I boot/include/ -o build/loader.o boot/loader.S
	
######################编译C内核代码###################################################
$(BUILD_DIR)/main.o:kernal/main.c
	$(CC) $(CFLAGS) -o $@ $<	
# $@表示规则中目标文件名的集合这里就是$(BUILD_DIR)/main.o  $<表示规则中依赖文件的第一个，这里就是kernle/main.c 

$(BUILD_DIR)/init.o:kernal/init.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/interrupt.o:kernal/interrupt.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/timer.o:device/timer.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/debug.o:kernal/debug.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/memory.o:kernal/memory.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/string.o:lib/string.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/bitmap.o:lib/kernal/bitmap.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/thread.o:thread/thread.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/list.o:lib/kernal/list.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/sync.o:thread/sync.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/console.o:device/console.c
	$(CC) $(CFLAGS) -o $@ $<

###################编译汇编内核代码#####################################################
$(BUILD_DIR)/kernal.o:kernal/kernal.S 
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/print.o:lib/kernal/print.S
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/switch_to.o:thread/switch.S
	$(AS) $(ASFLAGS) -o $@ $<

##################链接所有内核目标文件##################################################
$(BUILD_DIR)/kernal.bin:$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
# $^表示规则中所有依赖文件的集合，如果有重复，会自动去重

.PHONY:mk_dir hd clean build all boot	#定义了6个伪目标
mk_dir:
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi 
#判断build文件夹是否存在，如果不存在，则创建

hd:
	dd if=$(BUILD_DIR)/mbr.o of=$(HD60_PATH) count=1 bs=512 conv=notrunc && \
	dd if=$(BUILD_DIR)/loader.o of=$(HD60_PATH) count=4 bs=512 seek=2 conv=notrunc && \
	dd if=$(BUILD_DIR)/kernal.bin of=$(HD60_PATH) bs=512 count=200 seek=9 conv=notrunc
	
clean:
	@cd $(BUILD_DIR) && rm -f ./* && echo "remove ./build all done"
#-f, --force忽略不存在的文件，从不给出提示，执行make clean就会删除build下所有文件

build:$(BUILD_DIR)/kernal.bin
	
#执行build需要依赖kernel.bin，但是一开始没有，就会递归执行之前写好的语句编译kernel.bin

all:mk_dir boot build hd
#make all 就是依次执行mk_dir build hd