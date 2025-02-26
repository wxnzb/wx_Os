BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernal/ -I lib/user/ -I kernal/ -I device/ -I thread/
ASFLAGS = -f elf
CFLAGS = -Wall -m32 -fno-stack-protector $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS =  -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernal.map
OBJS = $(BUILD_DIR)/main.o  $(BUILD_DIR)/init.o   $(BUILD_DIR)/interrupt.o \
      $(BUILD_DIR)/timer.o  $(BUILD_DIR)/kernal.o $(BUILD_DIR)/print.o \
      $(BUILD_DIR)/debug.o  $(BUILD_DIR)/string.o $(BUILD_DIR)/memory.o \
      $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/thread.o 
##############     c代码编译     ###############
$(BUILD_DIR)/main.o: kernal/main.c lib/kernal/print.h \
        lib/stdint.h kernal/init.h lib/string.h kernal/memory.h \
    thread/thread.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernal/init.c kernal/init.h lib/kernal/print.h \
        lib/stdint.h kernal/interrupt.h device/timer.h kernal/memory.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernal/interrupt.c kernal/interrupt.h \
        lib/stdint.h kernal/global.h lib/kernal/io.h lib/kernal/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h\
        lib/kernal/io.h lib/kernal/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernal/debug.c kernal/debug.h \
        lib/kernal/print.h lib/stdint.h kernal/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c lib/string.h \
    kernal/debug.h kernal/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernal/memory.c kernal/memory.h \
    lib/stdint.h lib/kernal/bitmap.h kernal/debug.h lib/string.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernal/bitmap.c lib/kernal/bitmap.h \
    lib/string.h kernal/interrupt.h lib/kernal/print.h kernal/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h \
    lib/stdint.h lib/string.h kernal/global.h kernal/memory.h
	$(CC) $(CFLAGS) $< -o $@

##############    汇编代码编译    ###############
$(BUILD_DIR)/kernal.o: kernal/kernal.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernal/print.S
	$(AS) $(ASFLAGS) $< -o $@

##############    链接所有目标文件    #############
$(BUILD_DIR)/kernal.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd clean all

mk_dir:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi

hd:
	dd if=$(BUILD_DIR)/kernal.bin \
           of=/home/sweet/git/vc/JY/wx.img \
           bs=512 count=200 seek=9 conv=notrunc

clean:
	cd $(BUILD_DIR) && rm -f  ./*

build: $(BUILD_DIR)/kernal.bin

all: mk_dir build hd