# Copy from JamesM's kernel development tutorials: 
# (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html). 

# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel). 

# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=boot.o main.o

CFLAGS=-nostdlib -nostdinc -ffreestanding -fno-stack-protector
# -ffreestanding 按独立环境编译;
# 他隐含声明了`-fno-builtin'选项,而且对main函数没有特别要求.
# (译注:宿主环境(hosted environment)下所有的标准库可用,
# main函数返回一个int值,典型例子是除了内核以外几乎所有的程序.
# 对应的独立环境(freestanding environment)不存在标准库,
# 程序入口也不一定是 main,最明显的例子就是操作系统内核.
# 详情参考gcc网站最近的资料)

CC=gcc-7

LDFLAGS= -t link.ld

ASFLAGS=-felf

all: $(SOURCES) link

clean:
	-rm *.o kernel

link:
	$(CC) $(LDFLAGS) $(SOURCES) -o kernel

.s.o:
	nasm $(ASFLAGS) $<