/*
 * elfldr.c
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
 */

#include <arch/arch.h>
#include <elf.h>
#include <vfs.h>
#include <scheduler.h>
#include <printk.h>
#include <mmu.h>
#include <kheap.h>
#include <syscall.h>
#include <memory.h>

/*
 * load a program segment.
 */
static int elf_load_section(Elf32_Phdr *p_hdr, int fd)
{
	void *ptr;
	uint32_t vaddr;
	size_t sz;
	static int i = 0;	
	vaddr = p_hdr->p_vaddr;
	vaddr &= 0xFFFFF000;
	sz = p_hdr->p_memsz + (p_hdr->p_vaddr - vaddr);

	/*
	 * Since the two elf sections may overlap the same
	 * page we just request the buffer allocated at the next
	 * physical page. Since all sections are going to be mapped
	 * until the task is killed it shouldn't be a problem.
	 *
	 * We're assuming that the sections are ordered from lower
	 * to higher addresses. I don't know if this is always true
	 */
	while (1)
	{
		if (sz <= 0)
			break;
		if (!is_page_mapped(vaddr))
			break;
			
		vaddr += 0x1000;
		sz -= 0x1000;
	}
	
	assert(sz > 0);

	#if 1
	if (p_hdr->p_filesz > 0)
	{
		mmap((void*) vaddr, sz, PROT_READ | PROT_WRITE | PROT_EXEC, 
			MAP_PRIVATE, fd, p_hdr->p_offset);
	}
	else
	{
		ptr = (void*) kalloc(sz, vaddr, NULL, KALLOC_OPTN_ALIGN);
		if (ptr == NULL)
			return -1;
		assert(ptr == (void*) vaddr);
		
		ptr = (void*) p_hdr->p_vaddr;
		memset(ptr, 0, p_hdr->p_memsz);
	}
	#else
	/* allocate memory for section */
	ptr = (void*) kalloc(sz, vaddr, NULL, KALLOC_OPTN_ALIGN);
	if (ptr == NULL)
		return -1;
	assert(ptr == (void*) vaddr);
	
	ptr = (void*) p_hdr->p_vaddr;
	memset(ptr, 0, p_hdr->p_memsz);
	
	if (p_hdr->p_filesz > 0)
	{
		lseek(fd, p_hdr->p_offset, SEEK_SET);
		read(fd, (unsigned char*) ptr, p_hdr->p_memsz);
	}
	#endif
	return 0;
}

/*
 *
 */
intptr_t elf_load(int fd)
{
	intptr_t entry = NULL;
	unsigned int j;
	Elf32_Ehdr *elf_hdr;
	Elf32_Phdr *p_hdr;
	
	elf_hdr = malloc(sizeof(Elf32_Ehdr));
	if (elf_hdr == NULL)
		return NULL;

	/* read elf header */
	rewind(fd);
	read(fd, (unsigned char*) elf_hdr, sizeof(Elf32_Ehdr));
				
	/* check magic number */
	if (*((uint32_t*) elf_hdr->e_ident) != ELFMAG)
		return NULL;

	/* allocate memory for proggram headers */
	p_hdr = malloc(sizeof(Elf32_Phdr) * elf_hdr->e_phnum);
	if (p_hdr == NULL)
	{
		printk(7, "elf_loader: Out of memory");
		free(elf_hdr);
		return NULL;
	}

	/* read program headers */
	read(fd, (unsigned char*) p_hdr, sizeof(Elf32_Phdr) * elf_hdr->e_phnum);
	//vfs_read(fsnode, elf_hdr->e_phoff, 
	//	sizeof(Elf32_Phdr) * elf_hdr->e_phnum, (unsigned char*) p_hdr);

	/*
	 * load all section in order
	 */
	for (j = 0; j < elf_hdr->e_phnum; j++)
		if (p_hdr[j].p_type == PT_LOAD && p_hdr[j].p_memsz > 0)
			if (elf_load_section(&p_hdr[j], fd) == -1)
				printk(3, "elf_load: error loading section!");

	entry = elf_hdr->e_entry;
	free(p_hdr);
	free(elf_hdr);
	return entry;
}

