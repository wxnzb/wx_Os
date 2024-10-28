# 定义变量
NASM = nasm
GCC = gcc
LD = ld
DD = dd

# 定义目标文件和源文件
PRINT_SOURCE = lib/kernel/print.S
PRINT_OBJECT = lib/kernel/print.o
KERNEL_TARGET = kernel/kernel.bin
HD_IMAGE =/home/sweet/git/vc/JY/wx.img

# 默认规则：编译和安装
all: $(KERNEL_TARGET) install

# 编译 print.S 为目标文件
$(PRINT_OBJECT): $(PRINT_SOURCE)
	@mkdir -p lib/kernel
	$(NASM) -f elf -o $@ $<

# 链接目标文件生成 kernel.bin
$(KERNEL_TARGET): $(PRINT_OBJECT)
	@mkdir -p kernel
	$(LD) -m elf_i386 -Ttext 0xc0001500 -e main -o $@ $^

# 将编译好的内核写入虚拟硬盘
install: $(KERNEL_TARGET)
	$(DD) if=$(KERNEL_TARGET) of=$(HD_IMAGE) bs=512 count=200 seek=9 conv=notrunc

# 清理生成的文件
clean:
	rm -rf lib/kernel/*.o kernel/*.bin

.PHONY: all install clean