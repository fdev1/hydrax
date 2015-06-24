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

CC=../tools/bin/i386-hydrax-gcc
AS=../tools/bin/i386-hydrax-as
LD=../tools/bin/i386-hydrax-ld
ECHOFLAGS=-e

ifneq ($(shell echo -e "Hello"),Hello)
	ECHOFLAGS=
endif

CFLAGS=-g -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-DKERNEL_CODE -Iinclude -Wunused-variable
ASFLAGS=-ggdb --32
LDFLAGS=-melf_i386 -Tarch/x86/hydrax.ld -Map hydrax.map


SOURCES=arch/x86/boot.o \
	arch/x86/isr.o \
	arch/x86/arch.o \
	arch/x86/timer.o \
	arch/x86/video.o \
	arch/x86/scheduler.o \
	arch/x86/cpu.o \
	arch/x86/cmos.o \
	arch/x86/rtc.o \
	arch/x86/kbd.o \
	arch/x86/mmu.o \
	bget/bget.o \
	console.o \
	string.o \
	printk.o \
	timer.o \
	mmu.o \
	memory.o \
	isr.o \
	kheap.o \
	vfs.o \
	initrd.o \
	scheduler.o \
	symbols.o \
	idle.o \
	syscall.o \
	elfldr.o \
	devfs.o \
	tty.o \
	io.o \
	time.o \
	memmap.o \
	mutex.o \
	semaphore.o \
	procfs.o \
	signal.o \
	fork.o \
	env.o \
	null.o \
	zero.o \
	system.o \
	mman.o \
	main.o

all: kernel

clean: 
	rm -f *.o 
	rm -f arch/x86/*.o 
	rm -f bget/*.o 
	rm -f hydrax
	rm -f hydrax.map
	rm -f hydrax.syms
	rm -f initrd.img

kernel: $(SOURCES)  
	@echo $(ECHOFLAGS) "  LINK\t\thydrax"
	@$(LD) $(LDFLAGS) -o hydrax $(SOURCES)
	
	@echo $(ECHOFLAGS) "  SYMS\t\thydrax.map"
	@../tools/src/symclean/symclean hydrax.map hydrax.syms
	
	@echo $(ECHOFLAGS) "  MKINITRD\thydrax.initrd"
	@../tools/src/mkinitrd/mkinitrd \
		hydrax.syms /lib/hydrax.syms \
		../tools/src/mkinitrd/hello.txt /home/hello.txt \
		../utils/init /sbin/init \
		../utils/echo /bin/echo \
		../utils/chown /bin/chown \
		../utils/pipetest /bin/pipetest \
		../utils/pipetest /bin/threadtest \
		../files/inittab /etc/inittab \
		../files/shellrc /etc/shellrc > /dev/null
		
	@echo $(ECHOFLAGS) "  MKIMAGE\thydrax.img"
	@../tools/src/mkimage/mkimage.sh 

.s.o:
	@echo $(ECHOFLAGS) "  AS\t\t"$<
	@$(AS) $(ASFLAGS) -c $< -o $@

.c.o:
	@echo $(ECHOFLAGS) "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
	
	
vbox: kernel
	VirtualBox --dbg --startvm Hydrax 2> /dev/null &
	
run: kernel
	@echo -e "  QEMU\t\thydrax.img"
	@qemu-system-i386 -m 256M -hda ../hydrax.img 2> /dev/null 

debug: kernel
	qemu-system-i386 -S -s -m size=256 -hda ../hydrax.img 2> /dev/null &
	gdb --init-command=hydrax_gdbinit

debug-tui: kernel
	qemu-system-i386 -S -s -m size=256 -hda ../hydrax.img 2> /dev/null &
	gdb --tui --init-command=hydrax_gdbinit
	
withdebug: kernel
	qemu-system-i386 -S -s -m 256M -hda ../hydrax.img 2> /dev/null 

kdbg: kernel
	qemu-system-i386 -S -s -m 256M -hda ../hydrax.img 2> /dev/null &
	kdbg -r localhost:1234 hydrax &
	
