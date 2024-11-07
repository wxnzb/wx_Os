;-------------loader 和 kernel----------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2
PAGE_DIR_TABLE_POS equ 0x100000 
KERNEL_START_SECTOR equ 0x9
;为啥这里将elf文件放在0x70000?????不是很清楚
KERNEL_BIN_BASE_ADDR equ 0x70000                    ; 定义内核在内存中的缓冲区，也就是将编译好的内核文件暂时存储在内存中的位置                       ; 定义内核在磁盘的起始扇区
KERNEL_ENTER_ADDR equ 0xc0001500                    ; 定义内核可执行代码的入口地址

PT_NULL equ 0                                       ; -------------  程序段的 type 定义   --------------
                                                    ; 用于判断该段是否应该被加载

;--------------gdt 描述符属性 -------------
DESC_G_4K  equ 1_00000000000000000000000b
DESC_D_32 equ  1_0000000000000000000000b
DESC_L equ      0_000000000000000000000b
DESC_AVL equ     0_00000000000000000000b
DESC_LIMIT_CODE2 equ 1111_0000000000000000b
DESC_LIMIT_DATA2 equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2 equ 0000_0000000000000000b
DESC_P equ 1_000000000000000b
DESC_DPL_0 equ 00_0000000000000b
DESC_DPL_1 equ 01_0000000000000b
DESC_DPL_2 equ 10_0000000000000b
DESC_DPL_3 equ 11_0000000000000b
DESC_S_CODE equ  1_000000000000b
DESC_S_DATA equ DESC_S_CODE
DESC_S_sys equ   0_000000000000b
DESC_TYPE_CODE equ 1000_00000000b
DESC_TYPE_DATA equ 0010_00000000b

DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE +DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

;-------------- 选择子属性 ---------------
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
TI_GDT equ 000b
TI_LDT equ 100b

;-------启动分页机制！！！！！-------              ;---------模块化的页目录表字段,PWT PCD A D G AVL 暂时不用设置   ----------
PG_P  equ   1b
PG_RW_R	 equ  00b 
PG_RW_W	 equ  10b 
PG_US_S	 equ  000b 
PG_US_U	 equ  100b  
