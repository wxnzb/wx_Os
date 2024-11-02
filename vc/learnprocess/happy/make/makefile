# 1:2
# 	@echo "makefile test ok"
##########################
# t1:1
# 	@echo "target1"
# t2:1
# 	@echo "target2"
##########################
# .PHONY:clean
# 	rm ./build/*.o
##########################
# test2.o:test2.c
#     gcc -c -o test2.o test2.c
# test1.o:test1.c
#     gcc -c -o test1.o test1.c
#makefile你可真是聪明呢，这样都知道我在干啥
%.o:%.c
	gcc -c -o $@ $<
objfiles=test1.o test2.o

test.bin:$(objfiles)
# test.bin:test1.o test2.o
#	gcc -o test.bin test1.o test2.o
	gcc -o $@ $^
all:test.bin
	@echo "makefile test ok"