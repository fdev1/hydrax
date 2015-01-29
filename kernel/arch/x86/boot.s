#
# boot.s
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

.global start
.global mboot

.equ MBOOT_PAGE_ALIGN,	(1 << 0);
.equ MBOOT_MEM_INFO,	(1 << 1);
.equ MBOOT_HEADER_MAGIC,	(0x1BADB002)
.equ MBOOT_HEADER_FLAGS, (MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO)
.equ MBOOT_CHECKSUM,     (-(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS))

mboot:

	.int MBOOT_HEADER_MAGIC;
	.int MBOOT_HEADER_FLAGS;
	.int MBOOT_CHECKSUM;
	.int mboot;
	.int code;
	.int bss;
	.int end;
	.int start;

start:

	cli;
 	push %esp;
 	push %ebx;
 	call hydrax_init;
 1:
 	jmp 1b;
