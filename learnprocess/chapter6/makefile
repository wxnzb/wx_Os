NASM = nasm
NASMFLAGS_ELF = -f elf
GCC = gcc
LD = ld
NASMFLAGS = -f bin -I boot/include   # 指定输出格式为二进制，并设置包含路径

# 定义目标文件和源文件
PRINT_TARGET = build/print.o
KERNEL_TARGET = build/kernal.bin
MAIN_SOURCE = kernal/main.c
MAIN_TABLE=build/main.o
PRINT_SOURCE = lib/kernal/print.S
IMG = /home/sweet/git/vc/JY/wx.img
####################
MBR_TARGET = build/mbr.bin
MBR_SOURCE = boot/mbr.s
LOADER_SOURCE = boot/loader.S
LOADER_TARGET = build/loader.bin

# 默认规则
all: $(KERNEL_TARGET) $(LOADER_TARGET) $(MBR_TARGET) install
#为啥加上就运行不料，真的好奇怪！！！！,欸，怎么加上就可以运行了，good!!!
#编译 mbr.s 为二进制文件
$(MBR_TARGET): $(MBR_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 编译 loader.S 为二进制文件
$(LOADER_TARGET): $(LOADER_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 编译 print.S 为目标文件
$(PRINT_TARGET): $(PRINT_SOURCE)
	@mkdir -p   build         # 创建目录（如果不存在）
	$(NASM) $(NASMFLAGS_ELF) -o $@ $<  # 使用 ELF 格式

# 编译 main.c 为目标文件
$(MAIN_TABLE): $(MAIN_SOURCE)
	@mkdir -p build              # 创建目录（如果不存在）
	$(GCC) -m32 -I lib/kernal/ -c -o $@ $<

# 链接内核并生成最终的内核二进制
$(KERNEL_TARGET): build/main.o $(PRINT_TARGET)
	@mkdir -p build              # 创建目录（如果不存在）
	$(LD) -m elf_i386 -Ttext 0xc0001500 -e main -o $@ $^

# 将编译好的 mbr, loader, kernel 写入 wx.img
install: $(MBR_TARGET) $(LOADER_TARGET) $(KERNEL_TARGET)
	# dd if=$(MBR_TARGET) of=$(IMG) bs=512 count=1 conv=notrunc
	# dd if=$(LOADER_TARGET) of=$(IMG) bs=512 seek=2 count=3 conv=notrunc
	dd if=$(KERNEL_TARGET) of=$(IMG) bs=512 count=200 seek=9 conv=notrunc

# 清理生成的文件
clean:
	rm -rf build

.PHONY: all install clean
