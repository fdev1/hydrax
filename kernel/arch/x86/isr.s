#
# isr.s
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

.macro isr vector, err
	.global isr\()\vector;
	isr\()\vector :
	.if !\err
		push $0x0;
	.endif
	push $\vector;
	jmp isr_entry;
.endm

.macro irq, vector, irq
	.global irq\()\vector;
	irq\()\vector:
	push $0x0;
	push $\irq;
	jmp irq_entry;
.endm

.macro isr_noerr_vectors start=0, count=0
	isr_noerr "isr", 1, \start
	.if \count
		isr_noerr_vectors "(\start+1)", "(\count-1)"
	.endif
.endm

isr_entry:
	pusha;
	mov %ds, %ax;
	push %eax;
	cli;
	mov $0x10, %ax;
	mov %ax, %ds;
	mov %ax, %es;
	mov %ax, %fs;
	mov %ax, %gs;
	sti;
	call isr_handler;
	cli;
	pop %ebx;
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs
	sti;
	popa;
	add $0x8, %esp;
	iret;

irq_entry:
	pusha;
	mov %ds, %ax;
	push %eax;
	cli;
	mov $0x10, %ax;
	mov %ax, %ds;
	mov %ax, %es;
	mov %ax, %fs;
	mov %ax, %gs;
	sti;
	call irq_handler;
	cli;
	pop %ebx;
	mov %bx, %ds;
	mov %bx, %es;
	mov %bx, %fs;
	mov %bx, %gs;
	sti;
	popa;
	add $0x8, %esp;
	iret;

#
# ISR Stubs
#
isr   0,  0
isr   1,  0
isr   2,  0
isr   3,  0
isr   4,  0
isr   5,  0
isr   6,  0
isr   7,  0
isr   8,  1
isr   9,  0
isr  10,  1
isr  11,  1
isr  12,  1
isr  13,  1
isr  14,  1
isr  15,  0
isr  16,  0
isr  17,  0
isr  18,  0
isr  19,  0
isr  20,  0
isr  21,  0
isr  22,  0
isr  23,  0
isr  24,  0
isr  25,  0
isr  26,  0
isr  27,  0
isr  28,  0
isr  29,  0
isr  30,  0
isr  31,  0
isr  48,  0

#
# IRQ Stubs
#
irq   0, 32
irq   1, 33
irq   2, 34
irq   3, 35
irq   4, 36
irq   5, 37
irq   6, 38
irq   7, 39
irq   8, 40
irq   9, 41
irq  10, 42
irq  11, 43
irq  12, 44
irq  13, 45
irq  14, 46
irq  15, 47
