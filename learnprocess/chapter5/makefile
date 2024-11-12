#不是，5到底有什么问题啊，之前还可以运行的，真气死我了*……*
NASM = nasm
NASMFLAGS = -f bin -I boot/include   # 指定输出格式为二进制，并设置包含路径
GCC = gcc
LD = ld

# 定义目标文件和源文件
MBR_TARGET = build/mbr.bin
LOADER_TARGET = build/loader.bin
KERNEL_TARGET = build/kernel.bin
MBR_SOURCE = boot/mbr.s
LOADER_SOURCE = boot/3loader.S
KERNEL_SOURCE = kernal/main.c
IMG = /home/sweet/git/vc/JY/wx.img

# 默认规则：编译 mbr, loader, kernel 并将它们写入 wx.img
all: $(MBR_TARGET) $(LOADER_TARGET) $(KERNEL_TARGET) install

# 编译 mbr.s 为二进制文件
$(MBR_TARGET): $(MBR_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 编译 loader.S 为二进制文件
$(LOADER_TARGET): $(LOADER_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 编译 kernel (main.c) 为二进制文件
$(KERNEL_TARGET): $(KERNEL_SOURCE)
	$(GCC)  -m32 -c -o build/main.o $(KERNEL_SOURCE)
	$(LD) -m elf_i386 build/main.o -Ttext 0xc0001500 -e main -o $(KERNEL_TARGET)

# 将编译好的 mbr, loader, kernel 写入 wx.img
install: $(MBR_TARGET) $(LOADER_TARGET) $(KERNEL_TARGET)
	dd if=$(MBR_TARGET) of=$(IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADER_TARGET) of=$(IMG) bs=512 seek=2 count=3 conv=notrunc
	dd if=$(KERNEL_TARGET) of=$(IMG) bs=512 count=200 seek=9 conv=notrunc

# 清理生成的文件
clean:
	rm -rf build

.PHONY: all install clean

