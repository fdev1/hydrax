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
#include <unistd.h>

/*
 * load a program segment.
 */
static int elf_ldprog(Elf32_Phdr *p_hdr, int fd)
{
	void *ptr;
	uint32_t vaddr;
	size_t sz;
	static int i = 0;	
	vaddr = p_hdr->p_vaddr;
	vaddr &= 0xFFFFF000;
	sz = p_hdr->p_memsz + (p_hdr->p_vaddr & 0xFFF);

	assert(sz > 0);

	if (p_hdr->p_filesz > 0)
	{
		void *ap;
		ap = mmap((void*) p_hdr->p_vaddr, sz, PROT_READ | PROT_WRITE | PROT_EXEC, 
			MAP_SHARED, fd, p_hdr->p_offset);
		if (unlikely(ap == NULL))
		{
			printk(7, "ldprog: mmap failed!");
			return -1;
		}
		if (ap != (void*) vaddr)
			printk(7, "ldprog: ap = %x", ap);
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
	return 0;
}

#if 0
static int elf_ldsec(Elf32_Shdr *s_hdr, int fd, int i)
{
	uint32_t vaddr;
	size_t sz;
	const char *type;
	assert(s_hdr != NULL);
	/* assert(s_hdr->sh_addr != NULL); */
	
	if (s_hdr->sh_addr == NULL)
		return 0;
	
	switch (s_hdr->sh_type)
	{
		case SHT_STRTAB: type = "STRTAB"; break;
		case SHT_NOBITS: type = "NOBITS"; break;
		case SHT_PROGBITS: type = "PROGBITS"; break;
	}
	
	vaddr = s_hdr->sh_addr;
	vaddr &= 0xFFFFF000;
	sz = s_hdr->sh_size;
	assert(sz != 0);
	
	printk(7, "ldsec: loading section (%s) %i at %x", type, i, s_hdr->sh_addr);
	
	if (s_hdr->sh_type != SHT_NOBITS)
	{
		mmap((void*) vaddr, sz, PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_SHARED, fd, s_hdr->sh_offset);
	}
	else
	{
		void *ptr;
		ptr = (void*) kalloc(sz, vaddr, NULL, KALLOC_OPTN_ALIGN);
		if (unlikely(ptr == NULL))
			return -1;	/* TODO: Cleanup */
		assert(ptr == (void*) vaddr);
		ptr = (void*) s_hdr->sh_addr;
		memset(ptr, 0, s_hdr->sh_size);
	}
	return 0;	
}
#endif

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
	
	assert(elf_hdr->e_ident[EI_CLASS] == ELFCLASS32);

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

	/*
	 * load all program sections
	 */
	for (j = 0; j < elf_hdr->e_phnum; j++)
	{
		if (p_hdr[j].p_type == PT_LOAD && p_hdr[j].p_memsz > 0)
		{
			if (elf_ldprog(&p_hdr[j], fd) == -1)
				printk(3, "elf_load: error loading section!");
		}
	}

	entry = elf_hdr->e_entry;
	free(p_hdr);
	
	#if 0
	Elf32_Shdr *s_hdr;
	s_hdr = (Elf32_Shdr*) malloc(sizeof(Elf32_Shdr) * elf_hdr->e_shnum);
	if (unlikely(s_hdr == NULL))
	{
		current_task->errno = ENOMEM;
		free(elf_hdr);
		return NULL;
	}
	
	lseek(fd, elf_hdr->e_shoff, SEEK_SET);
	read(fd, (unsigned char*) s_hdr, sizeof(Elf32_Shdr) * elf_hdr->e_shnum);
	
	for (j = 0; j < elf_hdr->e_shnum; j++)
	{
		switch (s_hdr[j].sh_type)
		{
			case SHT_STRTAB:
			case SHT_NOBITS:
			case SHT_PROGBITS:
				if (elf_ldsec(&s_hdr[j], fd, j) == -1)
					printk(3, "elfld: error loading section %i", j);
				break;
			default:
				continue;
		}
	}
	free(s_hdr);
	#endif
	
	free(elf_hdr);
	return entry;
}

