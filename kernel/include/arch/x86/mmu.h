#ifndef __ARCH_MMU_H__
#define __ARCH_MMU_H__

#define MMU_PAGE_SIZE			(0x1000)

/*
 * Switch the current page directory.
 */
#define switch_page_directory(directory, physical)	\
	asm __volatile__(									\
		"cli;"									\
		"mov %0, current_directory;"					\
		"mov %1, %%cr3;"							\
		"mov %%cr0, %0;"							\
		"or $0x80010000, %0;"	/* PG | WP */			\
		"mov %0, %%cr0;" 							\
		"mov %%cr4, %0;"							\
		/* "or $0x100000, %0;" *//*CR4.SMAP=1*/			\
		"mov %0, %%cr4;"							\
		"sti;" : : "r" (directory), "r" (physical) : "memory" );

/*
 * Copy a physical page.
 */
#define copy_physical_page(dest, src)		\
	asm __volatile__(					\
		"cli;"						\
		"mov %%cr0, %%eax;"				\
		"and $0x7ffeffff, %%eax;"		\
		"mov %%eax, %%cr0;"				\
		"mov $0x100, %%eax;"			\
		"1:"							\
		"mov 0x0(%1), %%ebx;"			\
		"mov %%ebx, 0x0(%0);"			\
		"mov 0x4(%1), %%ebx;"			\
		"mov %%ebx, 0x4(%0);"			\
		"mov 0x8(%1), %%ebx;"			\
		"mov %%ebx, 0x8(%0);"			\
		"mov 0xC(%1), %%ebx;"			\
		"mov %%ebx, 0xC(%0);"			\
		"add $0x10, %0;"				\
		"add $0x10, %1;"				\
		"dec %%eax;"					\
		"jnz 1b;"						\
		"mov %%cr0, %%eax;"				\
		"or $0x80010000, %%eax;"			\
		"mov %%eax, %%cr0;"				\
		"sti;" : : "r" (dest), "r" (src) : "cc", "eax", "ebx")
		
#define arch_get_page_fault_address(addr)	asm __volatile__("mov %%cr2, %0" : "=r" (addr))
#define arch_invalidate_page(addr)			asm __volatile__("invlpg (%0)" : : "r" (addr));

/*
 * page entry
 */
typedef struct __attribute__((packed))
{
	uint32_t present:1;
	uint32_t rw:1;
	uint32_t user:1;
	uint32_t accessed:1;
	uint32_t dirty:1;
	uint32_t unused:7;
	uint32_t frame:20;
} 
page_t;

#endif
