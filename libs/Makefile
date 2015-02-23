#
# Makefile
#
# Author: Fernando Rodriguez
# Email: frodriguez.developer@outlook.com
# 
# Copyright 2014-2015 Fernando Rodriguez
# All rights reserved.
# 
# This code is distributed for educational purposes
# only. It may not be distributed without written 
# permission from the author.
# 

CC=../tools/bin/i386-elf-gcc
LD=../tools/bin/i386-hydrax-ld
AR=../tools/bin/i386-hydrax-ar

SOURCES=syscalls.o string.o stdio.o start.o bget/bget.o malloc.o

CFLAGS=-m32 -static -nostdlib -nostdinc -fno-builtin -I. -I../kernel/include -fno-stack-protector
ARFLAGS=mcs

all: libc 

clean: 
	rm -f *.o
	rm -f bget/*.o
	rm -f libc.a
	rm -f libpthread.a

libc: $(SOURCES)
	@echo -e "  AR\t\tlibc.a"
	@ar $(ARFLAGS) libc.a $(SOURCES)
	
.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@

