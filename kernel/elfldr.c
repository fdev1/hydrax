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
/*static*/ int elf_load_section(Elf32_Phdr *p_hdr, vfs_node_t *fsnode)
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

	if (sz > 0)
	{
		/* allocate memory for section */
		ptr = (void*) kalloc(sz, vaddr, NULL, KALLOC_OPTN_ALIGN);
		if (ptr == NULL)
			return -1;
		assert(ptr == (void*) vaddr);
	}
	//else
	//{
	//	vaddr = p_hdr->p_vaddr & 0xFFFFF000;
	//}
	
	page_t *tmp = mmu_get_page(p_hdr->p_vaddr, 0, current_directory);
	if (tmp != NULL)
		ptr = (void*) p_hdr->p_vaddr;
	memset(ptr, 0, p_hdr->p_memsz);
	
	if (p_hdr->p_filesz > 0)
		vfs_read(fsnode, p_hdr->p_offset, 
			p_hdr->p_memsz, (unsigned char*) ptr);
	return 0;
}

/*
 *
 */
intptr_t elf_load(vfs_node_t *fsnode)
{
	intptr_t entry = NULL;
	unsigned int j;
	Elf32_Ehdr *elf_hdr;
	Elf32_Phdr *p_hdr;

	elf_hdr = malloc(sizeof(Elf32_Ehdr));
	if (elf_hdr == NULL)
		return NULL;

	/* read elf header */
	vfs_read(fsnode, 0, sizeof(Elf32_Ehdr), (unsigned char*) elf_hdr);

	/* check magic number */
	if (	elf_hdr->e_ident[0] != 0x7F ||
		elf_hdr->e_ident[1] != 'E' ||
		elf_hdr->e_ident[2] != 'L' ||
		elf_hdr->e_ident[3] != 'F' )
		return NULL; /* invalid ELF file */

	#if 0
	printk(7, "elf_load: e_entry=0x%x", elf_hdr->e_entry);
	printk(7, "elf_load: e_machine=0x%x", elf_hdr->e_machine);
	printk(7, "elf_load: e_version=0x%x", elf_hdr->e_version);
	printk(7, "elf_load: e_type=0x%x", elf_hdr->e_type);
	printk(7, "elf_load: e_phnum=0x%x", elf_hdr->e_phnum);
	printk(7, "elf_load: e_phoff=0x%x", elf_hdr->e_phoff);
	printk(7, "==================================");
	#endif

	/* allocate memory for proggram headers */
	p_hdr = malloc(sizeof(Elf32_Phdr) * elf_hdr->e_phnum);
	if (p_hdr == NULL)
	{
		printk(7, "elf_loader: Out of memory");
		free(elf_hdr);
		return NULL;
	}

	/* read program headers */
	vfs_read(fsnode, elf_hdr->e_phoff, 
		sizeof(Elf32_Phdr) * elf_hdr->e_phnum, (unsigned char*) p_hdr);

	intptr_t last_section = 0x00000000;
	intptr_t next_section = 0xFFFFFFFF;

	/*
	 * load all section in order
	 */
	#if 1
	for (j = 0; j < elf_hdr->e_phnum; j++)
		if (p_hdr[j].p_type == PT_LOAD && p_hdr[j].p_memsz > 0)
			if (elf_load_section(&p_hdr[j], fsnode) == -1)
				printk(3, "elf_load: error loading section!");
	#else
	while (1)
	{
		next_section = 0xFFFFFFFF;
		for (j = 0; j < elf_hdr->e_phnum; j++)
			if (p_hdr[j].p_filesz || p_hdr[j].p_memsz)
				if (p_hdr[j].p_vaddr > last_section && p_hdr[j].p_vaddr < next_section)
					next_section = p_hdr[j].p_vaddr;

		if (next_section == 0xFFFFFFFF)
			break;

		for (j = 0; j < elf_hdr->e_phnum; j++)
			if (p_hdr[j].p_vaddr == next_section)
				if (p_hdr[j].p_memsz != 0)
					elf_load_section(&p_hdr[j], fsnode);
		last_section = next_section;

	}
	#endif

	entry = elf_hdr->e_entry;
	free(p_hdr);
	free(elf_hdr);
	return entry;
}

