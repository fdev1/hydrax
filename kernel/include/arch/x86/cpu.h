/*
 * cpu.h
 * 
 * Author: Fernando Rodriguez
 * Email: frodriguez.developer@outlook.com
 * 
 * Copyright 2014-2015 Fernando Rodriguez
 * All rights reserved.
 * 
 * This code is distributed for educational purposes
 * only. It may not be distributed without written 
 * permission from the author.
 *
 */

#ifndef __CPU_H__
#define __CPU_H__

#include <arch/arch.h>

/*
 * EFLAGS register structure
 */
typedef struct _eflags
{
	uint32_t CF:1;
	uint32_t reserved3:1;
	uint32_t PF:1;
	uint32_t reserved2:1;
	uint32_t AF:1;
	uint32_t rsvd1:1;
	uint32_t ZF:1;
	uint32_t SF:1;
	uint32_t TF:1;
	uint32_t IF:1;
	uint32_t IOPL:2;
	uint32_t NT:1;
	uint32_t RF:1;
	uint32_t VM:1;
	uint32_t AC:1;
	uint32_t VIF:1;
	uint32_t VIP:1;
	uint32_t ID:1;
	uint32_t reserved:10;
}
eflags_t;

/*
 * GDT entry structure
 */
typedef struct __gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} 
__attribute__((packed)) gdt_entry_t;

/*
 * GDT Pointer structure.
 */
typedef struct __gdt_ptr
{
	uint16_t limit;
	uint32_t base;
} 
__attribute__((packed)) gdt_ptr_t;

/*
 * IDT Entry structure
 */
typedef struct __idt_entry
{
	uint16_t base_lo;
	uint16_t sel;
	uint8_t always0;
	uint8_t flags;
	uint16_t base_hi;
} 
__attribute__((packed)) idt_entry_t;

/*
 * IDT Pointer structure
 */
typedef struct __idt_ptr
{
	uint16_t limit;
	uint32_t base;
} 
__attribute__((packed)) idt_ptr_t;

/*
 * TSS structure.
 */
typedef struct __tss_entry 
{
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} 
__attribute__((packed)) tss_entry_t;

extern tss_entry_t tss_entry;

/*
 * initialize the mmu
 */
void cpu_init(void);

/*
 * import isr vectors
 */
extern void isr0 (void);
extern void isr1 (void);
extern void isr2 (void);
extern void isr3 (void);
extern void isr4 (void);
extern void isr5 (void);
extern void isr6 (void);
extern void isr7 (void);
extern void isr8 (void);
extern void isr9 (void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr48(void);
extern void isr128(void);

extern void irq0 (void);
extern void irq1 (void);
extern void irq2 (void);
extern void irq3 (void);
extern void irq4 (void);
extern void irq5 (void);
extern void irq6 (void);
extern void irq7 (void);
extern void irq8 (void);
extern void irq9 (void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif

