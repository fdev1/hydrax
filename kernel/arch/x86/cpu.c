/*
 * cpu.c
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

#include <arch/platform.h>
#include <arch/cpu.h>
#include <printk.h>
#include <memory.h>

#define SS_TI_GDT			(0x00 << 3)
#define SS_TI_LDT			(0x01 << 3)
#define SS_RPL_KERNEL		(0x00)
#define SS_RPL_USER			(0x03)

#define SS_KERNEL_CODE		(0x01)
#define SS_KERNEL_DATA		(0x02)
#define SS_USER_CODE		(0x03)
#define SS_USER_DATA		(0x04)
#define SS_TSS				(0x05)

#define SS(ss, ti, rpl)		(ss << 3 | ti << 2 | rpl)

/*
 * Load the GDT
 */
#define gdt_flush(ptr)			\
	asm volatile(				\
		"lgdt (%0);"			\
		"mov $0x10, %%ax;"		\
		"mov %%ax, %%ds;"		\
		"mov %%ax, %%es;"		\
		"mov %%ax, %%fs;"		\
		"mov %%ax, %%gs;"		\
		"mov %%ax, %%ss;"		\
		"jmp $0x08, $1f;"		\
		"1:" : : "r" (ptr) : "eax" )

/*
 * Load the IDT
 */
#define idt_flush(ptr)			\
	asm volatile("lidt (%0)" : : "r" (ptr))
	
/*
 * Load the TSS
 */
#define tss_flush()				\
	asm volatile(				\
		"mov $0x2B, %%ax;"		\
		"ltr %%ax;" : : : "eax")
	
/*
 * Descriptors
 */
gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;
tss_entry_t tss_entry;

/*
 * Set a GDT entry
 */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_entries[num].base_low    = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high   = (base >> 24) & 0xFF;
	gdt_entries[num].limit_low   = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access      = access;
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
	idt_entries[num].base_lo = base & 0xFFFF;
	idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
	idt_entries[num].sel     = sel;
	idt_entries[num].always0 = 0;
	
	// We must uncomment the OR below when we get to using user-mode.
	// It sets the interrupt gate's privilege level to 3.
	idt_entries[num].flags   = flags | 0x60;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0)
{
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry_t);

	gdt_set_gate(num, base, limit, 0xE9, 0x00);
	memset(&tss_entry, 0, sizeof(tss_entry_t));

	tss_entry.ss0  = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = SS(SS_KERNEL_CODE, SS_TI_GDT, SS_RPL_USER);
	tss_entry.ss = SS(SS_KERNEL_DATA, SS_TI_GDT, SS_RPL_USER);
	tss_entry.ds = SS(SS_KERNEL_DATA, SS_TI_GDT, SS_RPL_USER);
	tss_entry.es = SS(SS_KERNEL_DATA, SS_TI_GDT, SS_RPL_USER);
	tss_entry.fs = SS(SS_KERNEL_DATA, SS_TI_GDT, SS_RPL_USER);
	tss_entry.gs = SS(SS_KERNEL_DATA, SS_TI_GDT, SS_RPL_USER);

	assert(tss_entry.cs == 0x0b);
	assert(tss_entry.ss == 0x13);
}

/*
 * Initialize GDT.
 */
static inline void init_gdt(void)
{
	gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
	gdt_ptr.base  = (uint32_t) &gdt_entries;
	gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
	write_tss(5, 0x10, 0x0);
	gdt_flush((uint32_t) &gdt_ptr);
	tss_flush();
}

/**
 * Initialize IDT.
 */
static inline void init_idt(void)
{
	idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
	idt_ptr.base  = (uint32_t) &idt_entries;
	memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

	/*
	 * Remap the IDT
	 */
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	/*
	 * Interrupt vectors
	 */
	idt_set_gate( 0, (uint32_t) isr0 , 0x08, 0x8E);
	idt_set_gate( 1, (uint32_t) isr1 , 0x08, 0x8E);
	idt_set_gate( 2, (uint32_t) isr2 , 0x08, 0x8E);
	idt_set_gate( 3, (uint32_t) isr3 , 0x08, 0x8E);
	idt_set_gate( 4, (uint32_t) isr4 , 0x08, 0x8E);
	idt_set_gate( 5, (uint32_t) isr5 , 0x08, 0x8E);
	idt_set_gate( 6, (uint32_t) isr6 , 0x08, 0x8E);
	idt_set_gate( 7, (uint32_t) isr7 , 0x08, 0x8E);
	idt_set_gate( 8, (uint32_t) isr8 , 0x08, 0x8E);
	idt_set_gate( 9, (uint32_t) isr9 , 0x08, 0x8E);
	idt_set_gate(10, (uint32_t) isr10, 0x08, 0x8E);
	idt_set_gate(11, (uint32_t) isr11, 0x08, 0x8E);
	idt_set_gate(12, (uint32_t) isr12, 0x08, 0x8E);
	idt_set_gate(13, (uint32_t) isr13, 0x08, 0x8E);
	idt_set_gate(14, (uint32_t) isr14, 0x08, 0x8E);
	idt_set_gate(15, (uint32_t) isr15, 0x08, 0x8E);
	idt_set_gate(16, (uint32_t) isr16, 0x08, 0x8E);
	idt_set_gate(17, (uint32_t) isr17, 0x08, 0x8E);
	idt_set_gate(18, (uint32_t) isr18, 0x08, 0x8E);
	idt_set_gate(19, (uint32_t) isr19, 0x08, 0x8E);
	idt_set_gate(20, (uint32_t) isr20, 0x08, 0x8E);
	idt_set_gate(21, (uint32_t) isr21, 0x08, 0x8E);
	idt_set_gate(22, (uint32_t) isr22, 0x08, 0x8E);
	idt_set_gate(23, (uint32_t) isr23, 0x08, 0x8E);
	idt_set_gate(24, (uint32_t) isr24, 0x08, 0x8E);
	idt_set_gate(25, (uint32_t) isr25, 0x08, 0x8E);
	idt_set_gate(26, (uint32_t) isr26, 0x08, 0x8E);
	idt_set_gate(27, (uint32_t) isr27, 0x08, 0x8E);
	idt_set_gate(28, (uint32_t) isr28, 0x08, 0x8E);
	idt_set_gate(29, (uint32_t) isr29, 0x08, 0x8E);
	idt_set_gate(30, (uint32_t) isr30, 0x08, 0x8E);
	idt_set_gate(31, (uint32_t) isr31, 0x08, 0x8E);

	/*
	 * IRQ vectors
	 */
	idt_set_gate(32, (uint32_t) irq0, 0x08, 0x8E);
	idt_set_gate(33, (uint32_t) irq1, 0x08, 0x8E);
	idt_set_gate(34, (uint32_t) irq2, 0x08, 0x8E);
	idt_set_gate(35, (uint32_t) irq3, 0x08, 0x8E);
	idt_set_gate(36, (uint32_t) irq4, 0x08, 0x8E);
	idt_set_gate(37, (uint32_t) irq5, 0x08, 0x8E);
	idt_set_gate(38, (uint32_t) irq6, 0x08, 0x8E);
	idt_set_gate(39, (uint32_t) irq7, 0x08, 0x8E);
	idt_set_gate(40, (uint32_t) irq8, 0x08, 0x8E);
	idt_set_gate(41, (uint32_t) irq9, 0x08, 0x8E);
	idt_set_gate(42, (uint32_t) irq10, 0x08, 0x8E);
	idt_set_gate(43, (uint32_t) irq11, 0x08, 0x8E);
	idt_set_gate(44, (uint32_t) irq12, 0x08, 0x8E);
	idt_set_gate(45, (uint32_t) irq13, 0x08, 0x8E);
	idt_set_gate(46, (uint32_t) irq14, 0x08, 0x8E);
	idt_set_gate(47, (uint32_t) irq15, 0x08, 0x8E);

	/*
	 * System call vector
	 */
	idt_set_gate(48, (uint32_t) isr48, 0x08, 0x8E);

	/*
	 * load the IDT
	 */
	idt_flush((uint32_t) &idt_ptr);
}

/*
 * initialize cpu
 */
void cpu_init(void)
{
	init_gdt();
	init_idt();
	printk(7, "cpu: ready...");
}

