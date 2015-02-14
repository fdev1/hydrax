/*
 * include/mmu.h
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

#ifndef __MMU_H__	
#define __MMU_H__

#include <arch/arch.h>
#include <arch/mmu.h>
#include <isr.h>
#include <multiboot.h>
#include <memmap.h>

/*
 * kalloc options   
 * TODO: The codes need to be updated so that they
 * match the ones for ALOC_FRAME_XXX bellow (which should
 * actually be ALLOC_PAGE_XXX). That way we can just pass
 * the kalloc flags (perhaps after masking some bits) 
 * to mmu_alloc_page().
 */
#define KALLOC_OPTN_ALIGN		(0x0001)  /* page aligned */
#define KALLOC_OPTN_KERNEL		(0x0002)  /* kernel only */
#define KALLOC_OPTN_ONDEMAND		(0x0004)  /* allocate frames on demand */
#define KALLOC_OPTN_WRITEABLE		(0x0008)  /* writeable from userland */
#define KALLOC_OPTN_NOFREE		(0x0010)  /* pages will never be freed (dont track) */
#define KALLOC_OPTN_COW			(0X0020)  /* LAZY ALLOCATION commit on write */
#define KALLOC_OPTN_PUBLIC		(0x0042)  /* invisible from userspace */
#define KALLOC_OPTN_NOCOW		(0x0080)  /* no commit on write */
#define KALLOC_OPTN_NOSHARE		(0x0100)	/* do not share with child threads */
#define KALLOC_OPTN_MMAP			(0x0200)	/* memory used by mmap */
#define KALLOC_OPTN_CONSEC		(0x0400)	/* allocate consecutive physical memory */

/*
 * mmu_clone_directory options
 */
#define CLONE_COPY				(0x00)
#define CLONE_SHARE				(0x01)
#define CLONE_COW				(0x02)

/*
 * mmu_alloc_frame flags
 */
#define ALLOC_FRAME_USER			(0x0000)
#define ALLOC_FRAME_KERNEL		(0x0001)	/* allocate page on the kernel directory */
#define ALLOC_FRAME_WRITEABLE		(0x0002)	/* page is writeable */
#define ALLOC_FRAME_MKTABLE		(0x0004)	/* create page table if necessary */
#define ALLOC_FRAME_COW			(0x0008)	/* allow COW when cloning */
#define ALLOC_FRAME_NOSHARE		(0x0010)	/* do not share with threads */
#define ALLOC_FRAME_MMAP			(0x0020)	/* mmaped */
#define ALLOC_FRAME_ALLOC_PHYS	(0x0040)	/* allocate a physical page and map it */

/*
 * Structure used to track allocated pages.
 */
typedef struct __buffer
{
	intptr_t address;
	uint32_t pages;
	struct __buffer *next;
}
buffer_t;

/*
 * Holds arch independent info about
 * each page on a page directory.
 */
typedef struct 
{
	unsigned int commited:1;
	unsigned int cow:1;
	unsigned int noshare:1;
	unsigned int mmap:1;
	unsigned int pad:28;
}
page_info_t;

/*
 * page table entry.
 * By using a separate type for this we let the
 * compiler do some of the math for us.
 *
 */
typedef struct page_table
{
    page_t pages[1024];
    page_info_t pages_info[1024];
    unsigned int refs;
    unsigned int noshare:1;
    unsigned int padding:31;
} 
page_table_t;

/*
 * page directory structure
 */
typedef struct page_directory
{
	page_table_t *tables[1024];
	uint32_t tablesPhysical[1024];
} 
page_directory_t;

/*
 * Imports
 */
extern page_directory_t *current_directory;
extern intptr_t current_directory_physical;
extern memmap_t *current_memmap;

/*
 * initialize mmu
 */
void mmu_init(multiboot_header_t *p_mboot);

/*
 * Gets the physical memmap
 */
memmap_t *mmu_get_physical_map(void);

/*
 * Switch to the keernel's directory
 */
void mmu_switch_to_kernel_directory(void);

/*
 * Allocates enough pages for a buffer of size sz and maps the pages
 * to physical memory.
 * 
 * If virt is not NULL the page is allocated at the address specified by it.
 * If phys is not NULL the location pointed by it is set to the physical
 * address that the page was mapped to.
 * If the KALLOC_OPTN_KERNEL is specified
 */
intptr_t kalloc(uint32_t sz, uint32_t virt, uint32_t *phys, uint32_t flags);

/*
 * free memory allocated by kalloc
 */
void kfree(void*);

/*
 * Clone a page directory
 */
page_directory_t *mmu_clone_directory(page_directory_t *src, intptr_t *physical, uint32_t flags);

/*
 * Destroy a page directory
 */
void mmu_destroy_directory(page_directory_t *directory);

#endif

