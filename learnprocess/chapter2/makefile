# 定义编译器和选项
NASM = nasm
NASMFLAGS = -f bin  # 指定输出格式为二进制

# 定义目标文件和源文件
MBR_TARGET = build/mbr.bin
MBR_SOURCE = boot/1mbr.s
IMG = /home/sweet/git/vc/JY/wx.img  # 输出的镜像文件

# 默认规则：编译 mbr，并将其写入 wx.img
all: $(MBR_TARGET) install

# 编译 mbr.s 为二进制文件
$(MBR_TARGET): $(MBR_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 将编译好的 mbr 写入 wx.img 的第一个扇区
install: $(MBR_TARGET)
	dd if=$(MBR_TARGET) of=$(IMG) bs=512 count=1 conv=notrunc

# 清理生成的文件
clean:
	rm -rf build

.PHONY: all install clean
