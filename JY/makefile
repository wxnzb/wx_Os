# 定义编译器和选项
NASM = nasm
NASMFLAGS = -f bin -I boot/include   # 指定输出格式为二进制，并设置包含路径

# 定义目标文件和源文件
MBR_TARGET = build/mbr
LOADER_TARGET = build/loader
MBR_SOURCE = boot/mbr.S
LOADER_SOURCE = boot/loader.S
IMG = wx.img

# 默认规则：编译 mbr 和 loader，并将它们写入 wx.img
all: $(MBR_TARGET) $(LOADER_TARGET) install

# 编译 mbr.S 为二进制文件
$(MBR_TARGET): $(MBR_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 编译 loader.S 为二进制文件
$(LOADER_TARGET): $(LOADER_SOURCE)
	@mkdir -p build          # 创建 build 目录（如果不存在）
	$(NASM) $(NASMFLAGS) -o $@ $<

# 将编译好的 mbr 写入 wx.img 的第一个扇区
# 将编译好的 loader 写入 wx.img 的第二个扇区
install: $(MBR_TARGET) $(LOADER_TARGET)
	dd if=$(MBR_TARGET) of=$(IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADER_TARGET) of=$(IMG) bs=512 seek=2 count=1 conv=notrunc

# 清理生成的文件
clean:
	rm -rf build
	rm -f $(IMG)

.PHONY: all install clean
