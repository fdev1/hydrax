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

SOURCES=unistd.o string.o stdio.o start.o
PTHREAD=pthread.o

CFLAGS=-g -m32 -static -nostdlib -nostdinc -fno-builtin -I. -I../kernel/include -fno-stack-protector
ARFLAGS=mcs

all: libc pthread

clean: 
	rm -f *.o 
	rm -f libc.a
	rm -f libpthread.a

libc: $(SOURCES)
	@echo -e "  AR\t\tlibc.a"
	@ar $(ARFLAGS) libc.a $(SOURCES)
	
pthread: $(PTHREAD)
	@echo -e "  AR\t\tlibpthread.a"
	@ar $(ARFLAGS) libpthread.a $(PTHREAD)

.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@

