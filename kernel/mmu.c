/*
 * mmu.c
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

#if defined(KERNEL_CODE)
#undef KERNEL_CODE
#endif
#define KERNEL_CODE

#include <arch/arch.h>
#include <mmu.h>
#include <printk.h>
#include <memory.h>
#include <kheap.h>
#include <symbols.h>
#include <memmap.h>
#include <unistd.h>
#include <mutex.h>
#include <scheduler.h>
#include <errno.h>
#include <signal.h>
#include <io.h>

#define MMU_CR0_PG				(0x0000)
#define MMU_CR4_PG				(0x0000)

/*
 * These are used to enable debug output
 */
#define MMU_DEBUG_KALLOC			(0)
#define MMU_DEBUG_CLONE			(0)	
#define MMU_DEBUG_CLONE_TABLE		(0)
#define MMU_DEBUG_KFREE			(0)

/*
 * How much virtual memory is reserved
 * for the kernel
 */
#define MMU_GLOBAL_SPACE			(0x400000)


/*
 * Structure used by the MMU driver to keep
 * track of file mappings.
 */
typedef struct __kernel_mmap
{
	mmap_info_t *mmap;			/* master mmap */
	vfs_node_t *node;			/* vfs node mapped */
	page_directory_t *directory;	/* directory containing the master mapping */
	struct __kernel_mmap *next;	/* next */
}
kmmap_t;

/*
 * imports
 */
extern uint32_t ucode;		/* defined by linker */
extern uint32_t data;		/* defined by linker */
extern uint32_t end;    		/* defined by linker */
extern kheap_t kheap;

/*
 * static members
 */
static __attribute__((aligned(0x1000))) page_directory_t kernel_directory;
static memmap_t physical_memmap;
static memmap_t kernel_memmap;
static memmap_t default_memmap;
static uint32_t placement_address = (uint32_t) &end;
static uint32_t *frame_refs;
static mutex_t frame_refs_lock = MUTEX_INITIALIZER;
static buffer_t *kernel_buffers = NULL;
static kmmap_t *kmmaps = NULL;
static mutex_t kmmaps_lock = MUTEX_INITIALIZER;

/*
 * Globals
 */
page_directory_t *current_directory = NULL;
intptr_t current_directory_physical = NULL;
memmap_t *current_memmap = NULL;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))


/*
 * Gets the physical memmap
 */
memmap_t *mmu_get_physical_map(void)
{
	return &physical_memmap;
}

/*
 * Free physical page.
 */
static void mmu_frame_free(page_t *page)
{
	assert(page->frame != 0x0);

	mutex_wait(&frame_refs_lock);
	if (--frame_refs[page->frame] == 0)
	{
		mutex_busywait(&physical_memmap.lock);
		memmap_clear_page(&physical_memmap, page->frame * MMU_PAGE_SIZE);
		mutex_release(&physical_memmap.lock);
	}
	mutex_release(&frame_refs_lock);
	
	page->frame = 0x0;
	page->present = 0;
	page->accessed = 0;
	page->dirty = 0;
	page->rw = 0;
	page->user = 0;
}


/*
 * Allocates a physical frame for a page. If the page is already
 * mapped to a physical address it return 0x0, otherwise it returns
 * the physical address of the page
 */
static uint32_t mmu_alloc_frame(uint32_t address, 
	page_directory_t *directory, unsigned int flags)
{
	const uint32_t addr = address / MMU_PAGE_SIZE;
	const uint32_t indx = addr / 1024;
	const uint32_t offs = addr % 1024;
	page_t *page;
	page_info_t *page_info;
	
	assert(directory != NULL);
	assert(directory->tables[indx] != NULL);
	
	page = &directory->tables[indx]->pages[offs];
	page_info = &directory->tables[indx]->pages_info[offs];
	
	if (page->frame != 0)
	{
		return NULL;	/* TODO: Should we assert here? */
	}
	else
	{
		uint32_t idx;
		
		mutex_busywait(&physical_memmap.lock);
		idx = memmap_find_free_page(&physical_memmap);
		if ( idx == (uint32_t) -1 )
		{
			panic("out of frames");
		}
		memmap_set_page(&physical_memmap, idx * MMU_PAGE_SIZE);
		mutex_release(&physical_memmap.lock);
		page->present = 1;
		page->accessed = 0;
		page->dirty = 1;
		page->rw = (flags & ALLOC_FRAME_WRITEABLE) ? 1 : 0;
		page->user = (flags & ALLOC_FRAME_KERNEL) ? 0 : 1;
		page->frame = idx;
		assert(page_info != NULL);
		page_info->commited = 1;
		frame_refs[idx]++;
		
		return idx * MMU_PAGE_SIZE;
	}
}

/*
 * If kalloc() is called before the mmu is initialized
 * the call gets redirected to this function.
 * Right now this is only done when allocated the memmap
 * for physical pages (in mmu_init()).
 */
static intptr_t kalloc_premmu(uint32_t sz, uint32_t virt, uint32_t *phys, uint32_t flags)
{
	uint32_t tmp;

	assert(flags & KALLOC_OPTN_KERNEL);

	/*
	 * Make sure address is word aligned.
	 */
	if (placement_address & 0x3)
	{
		placement_address &= 0xFFFFFFFC;
		placement_address += 0x4;
	}

	/*
	 * if the page is already mapped
	 */
	if ((sz < (placement_address & 0xFFFFF000) + 0x1000) && 
		(flags & KALLOC_OPTN_ALIGN) == 0)
	{
		tmp = placement_address;
		placement_address += sz;
		return tmp;
	}

	if ((flags & KALLOC_OPTN_ALIGN) && (placement_address & 0xFFF) )
	{
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	
	if (phys)
		*phys = placement_address;

	tmp = placement_address;
	placement_address += sz;
	return tmp;
}

/*
 * Allocates enough pages for a buffer of size sz and maps the pages
 * to physical memory.
 * 
 * If virt is not NULL the page is allocated at the address specified by it.
 * If phys is not NULL the location pointed by it is set to the physical
 * address that the page was mapped to.
 * If the KALLOC_OPTN_KERNEL is specified the page is allocated in
 * the kernel directory.
 * 
 * If the KALLOC_OPTN_MMAP is specified the page is allocated and marked
 * as a memory mapped file page but it is not mapped. The mapping will happen
 * when the page is accessed.
 * 
 * TODO: This needs to be cleaned up soon as it's getting messy.
 */
intptr_t kalloc(uint32_t sz, uint32_t vaddr, uint32_t *phys, uint32_t flags)
{
	uint32_t physmem;
	page_directory_t *dir;
	memmap_t *memmap;
	intptr_t tmp_vaddr;
	buffer_t **buffers;
	
	if (phys == NULL)
		phys = &physmem;
	
	if (unlikely(current_directory == NULL))
		return kalloc_premmu(sz, vaddr, phys, flags);
	
	/* make sure that the virtual address is aligned */
	assert(vaddr == NULL || (vaddr & 0xFFFFF000) == vaddr);

	/*
	 * If the KALLOC_OPTN_KERNEL option was specified
	 * we use the kernel memmap and the kernel directory
	 * for this allocation. Otherwise we use the current
	 * usermode memmap and directory.
	 */
	if (flags & KALLOC_OPTN_KERNEL)
	{
		memmap = &kernel_memmap;
		dir = &kernel_directory;
		buffers = &kernel_buffers;
	}
	else
	{
		memmap = current_memmap;
		dir = current_directory;
		buffers = &current_task->buffers;
	}
	
	/*
	 * lock the memmap and attempt to allocate the requested
	 * pages from it.
	 */
	mutex_wait(&memmap->lock);
	if (vaddr != NULL)
	{
		assert((vaddr & 0xFFF) == 0);
		for (tmp_vaddr = vaddr; tmp_vaddr < vaddr + sz; tmp_vaddr += 0x1000)
		{
			if (memmap_test_page(memmap, tmp_vaddr))
			{
				mutex_release(&memmap->lock);
				return NULL;
			}
		}
	}
	else
	{
		vaddr = memmap_find_free_pages(memmap, (sz + 0xFFF) / 0x1000) * 0x1000;
		if (unlikely(vaddr == NULL))
		{
			mutex_release(&memmap->lock);
			return NULL;
		}
	}

	/*
	 * Mark all the pages requested as used and release
	 * the lock
	 */
	for (tmp_vaddr = vaddr; tmp_vaddr < vaddr + sz; tmp_vaddr += 0x1000)
		memmap_set_page(memmap, tmp_vaddr);
	mutex_release(&memmap->lock);

	
	page_t *page;
	page_info_t *page_info;
	uint32_t addr = vaddr / MMU_PAGE_SIZE;
	const uint32_t cow = (flags & KALLOC_OPTN_NOCOW) ? 0 : 1;
	
	/*
	 * Allocate and map all pages requested.
	 */
	page = mmu_get_page(vaddr, 1, dir);
	assert(page != NULL);
	page_info = &dir->tables[addr / 1024]->pages_info[addr % 1024];
	page_info->cow = cow;
	page_info->noshare = ((flags & KALLOC_OPTN_NOSHARE) != 0);
	page_info->mmap = ((flags & KALLOC_OPTN_MMAP) != 0);
	if (page_info->noshare == 1)
		dir->tables[addr / 1024]->noshare = 1;
	if (likely(page_info->mmap == 0))
		*phys = mmu_alloc_frame(vaddr, dir, ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
	else
		*phys = 0;
	
	for (tmp_vaddr = vaddr + 0x1000; tmp_vaddr < vaddr + sz; tmp_vaddr += 0x1000)
	{
		addr = tmp_vaddr / MMU_PAGE_SIZE;
		page = mmu_get_page(tmp_vaddr, 1, dir);
		assert(page != NULL);
		page_info->cow = cow;
		page_info = &dir->tables[addr / 1024]->pages_info[addr % 1024];
		page_info->noshare = ((flags & KALLOC_OPTN_NOSHARE) != 0);
		page_info->mmap = ((flags & KALLOC_OPTN_MMAP) != 0);
		if (page_info->noshare == 1)
			dir->tables[addr / 1024]->noshare = 1;
		if (likely(page_info->mmap == 0))
			mmu_alloc_frame(tmp_vaddr, dir, ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
	}

	if ((flags & KALLOC_OPTN_NOFREE) == 0 && kheap.len)
	{
		buffer_t **tmp;
		buffer_t *buf = (buffer_t*) malloc(sizeof(buffer_t));
		assert(buf != NULL);
		buf->address = vaddr;
		buf->pages = (sz + 0xFFF) / 0x1000;
		buf->next = NULL;

		while (*buffers != NULL)
			buffers = &(*buffers)->next;
		*buffers = buf;
	}

	return vaddr;
}

/*
 * free memory allocated by kalloc
 */
void kfree(void* ptr)
{
	unsigned int i;
	intptr_t addr;
	page_directory_t *dir;
	memmap_t *memmap;
	
	buffer_t **tmp, **buf, *parent;
	tmp = &kernel_buffers;
	buf = tmp;
	parent = NULL;
	dir = &kernel_directory;
	memmap = &kernel_memmap;
	while(*tmp != NULL && (*tmp)->address != (intptr_t) ptr)
	{
		parent = *tmp;
		tmp = &(*tmp)->next;
	}
	
	if (*tmp == NULL)
	{
		assert(current_task != NULL);
		tmp = &current_task->buffers;
		buf = tmp;
		parent = NULL;
		dir = current_directory;
		memmap = arch_get_memmap(current_task->machine_state);
		while(*tmp != NULL && (void*) (*tmp)->address != ptr)
		{
			parent = *tmp;
			tmp = &(*tmp)->next;
		}
	}

	if (*tmp == NULL)
		asm("nop");
	
	assert(*tmp != NULL);

	addr = (intptr_t) (*tmp)->address;
	for (i = 0; i < (*tmp)->pages; i++)
	{
		mmu_frame_free(mmu_get_page(addr, 0, dir));
		memmap_clear_page(memmap, addr);
		addr += 0x1000;
	}	
	
	if (parent != NULL)
	{
		buffer_t *tmpbuf;
		tmpbuf = *tmp;
		parent->next = (*tmp)->next;
		free((void*) tmpbuf);
	}
	else
	{
		void *tmpmem;
		tmpmem = *tmp;
		*buf = (*tmp)->next;
		free(tmpmem); /* free from heap */
	}
}

void mmu_switch_to_kernel_directory(void)
{
	current_memmap = &default_memmap;
	switch_page_directory(&kernel_directory, 
		(uint32_t) kernel_directory.tablesPhysical);
}

/*
 * dumps a page entry
 */
void mmu_dump_page(intptr_t virt)
{
	uint32_t table_idx;
	virt /= 0x1000;
	table_idx = virt / 1024;

	if (current_directory->tables[table_idx])
	{
		page_t *page = &current_directory->tables[table_idx]->pages[virt % 1024];
		printk(8, "mmu_dump_page (0x%x): idx=0x%x", virt * 0x1000, page->frame);
		printk(8, "mmu_dump_page (0x%x): present=%i", virt * 0x1000, page->present);
		printk(8, "mmu_dump_page (0x%x): rw=%i", virt * 0x1000, page->rw);
		printk(8, "mmu_dump_page (0x%x): user=%i", virt * 0x1000, page->user);
	}
	else
	{
		printk(8, "mmu_dump_page: no page for 0x%x", virt << 12);
	}
}

/*
 * Make a physical page.
 * Why can't this be done by kalloc?
 */
void mmu_make_page(uint32_t address)
{
	mmu_get_page(address, 1, current_directory);
}

/*
 * determines whether a page is mapped on the
 * current directory
 */
int is_page_mapped(intptr_t address)
{
	page_t *p = mmu_get_page(address, 0, current_directory);
	if (p == NULL || p->frame == NULL)
		return 0;
	return 1;		
}

/*
 * gets a page structure from the specified directory
 */
page_t *mmu_get_page(uint32_t address, int make, page_directory_t *dir)
{
	uint32_t table_idx;
	address /= MMU_PAGE_SIZE;
	table_idx = address / 1024;

	if (dir->tables[table_idx])
	{
		return &dir->tables[table_idx]->pages[address % 1024];
	}
	else if (make)
	{
		uint32_t tmp;
		dir->tables[table_idx] = (page_table_t*) kalloc(sizeof(page_table_t), 
			NULL, &tmp, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);
		if (dir->tables[table_idx] == NULL)
			return NULL;
		memset(dir->tables[table_idx], 0, sizeof(page_table_t));
		dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
		return &dir->tables[table_idx]->pages[address % 1024];
	}
	else
	{
		return NULL;
	}
}

/*
 * page fault handler
 */
void page_fault(registers_t *regs)
{
	intptr_t address;
	page_t *page;
	page_info_t *page_info;
	uint32_t *refs;
	
	const unsigned int p = !(regs->err_code & 0x01) >> 0;
	const unsigned int rw = (regs->err_code & 0x02) >> 1;
	const unsigned int us = (regs->err_code & 0x04) >> 2;
	const unsigned int rs = (regs->err_code & 0x08) >> 3;
	const unsigned int id = (regs->err_code & 0x10) >> 4;
	
	arch_get_page_fault_address(address);
	
	const uint32_t addr = address / MMU_PAGE_SIZE;
	const uint32_t indx = addr / 1024;
	const uint32_t offs = addr % 1024;

	page = &current_directory->tables[indx]->pages[offs];
	page_info = &current_directory->tables[indx]->pages_info[offs];
	refs = &frame_refs[page->frame];
	
	if (page != NULL && page_info->commited == 0)
	{
		/*
		 * The page is mapped to a file that has not been
		 * mapped to a physical frame.
		 * 
		 * TODO: For now the 1st access to a mmaped file
		 * will cause the whole file to be read into physical
		 * memory.
		 * 
		 */
		if (page_info->mmap == 1 && page->frame == 0)
		{
			mmap_info_t *mmap;
			kmmap_t *kmmap, **pkmmap;
			file_t *f;

			mutex_wait(&current_task->mmaps_lock);
			mmap = current_task->mmaps;
			while (mmap != NULL)
			{
				if (unlikely(address >= (intptr_t) mmap->addr && 
					address <= ((intptr_t) mmap->addr + mmap->len)))
					break;
				mmap = mmap->next;
			}
			mutex_release(&current_task->mmaps_lock);
			
			if (unlikely(mmap == NULL))
				goto unhandled_fault;
			
			f = get_file_descriptor(mmap->fd);
			if (unlikely(f == NULL))
				goto unhandled_fault;
			
			/*
			 * Look for an existing mapping for the same
			 * file node and offset
			 */
			mutex_wait(&kmmaps_lock);
			for (kmmap = kmmaps; kmmap != NULL; kmmap = kmmap->next)
				if (kmmap->node == f->node && mmap->offset == kmmap->mmap->offset)
					break;
			
			if (kmmap != NULL)	/* if there's an existing mapping... */
			{
				off_t f_offset;
				intptr_t m_vaddr, c_vaddr;
				page_t *c_page, *m_page;
				page_info_t *c_page_info, *m_page_info;
				const uint32_t c_addr = (intptr_t) mmap->addr / MMU_PAGE_SIZE;
				uint32_t c_indx = addr / 1024;
				uint32_t c_offs = addr % 1024;
				const uint32_t m_addr = (intptr_t) kmmap->mmap->addr / MMU_PAGE_SIZE;
				uint32_t m_indx = addr / 1024;
				uint32_t m_offs = addr % 1024;
				
				f_offset = mmap->offset;
				c_page = &current_directory->tables[c_indx]->pages[c_offs];
				c_page_info = &current_directory->tables[c_indx]->pages_info[c_offs];
				m_page = &kmmap->directory->tables[m_indx]->pages[c_offs];
				m_page_info = &kmmap->directory->tables[m_indx]->pages_info[c_offs];
				
				/*
				 * Copy all the mappings from the master
				 * mapping directory.
				 */
				c_vaddr = (intptr_t) mmap->addr;
				m_vaddr = (intptr_t) kmmap->mmap->addr;
				while (c_vaddr < (intptr_t) mmap->addr + mmap->len)
				{
					mutex_wait(&frame_refs_lock);
					frame_refs[m_page->frame]++;
					mutex_release(&frame_refs_lock);

					/*
					 * If the master page is writeable then
					 * make mark it as read only and clear the
					 * commited flag.
					 * 
					 * NOTE: This won't be a problem with any
					 * of our programs yet but it means that if
					 * the 1st file to map the file modified we
					 * get the modified version even if it's not
					 * or will never be flushed to disk.
					 */
					if (m_page->rw == 1)
					{
						m_page->rw = 0;
						m_page_info->commited = 0;
					}
					
					*c_page = *m_page;
					*c_page_info = *m_page_info;	
					arch_invalidate_page(c_vaddr);
					c_vaddr += MMU_PAGE_SIZE;
					m_vaddr += MMU_PAGE_SIZE;
					f_offset += MMU_PAGE_SIZE;
					
					c_page++;
					m_page++;
					c_page_info++;
					m_page_info++;
					
					if (unlikely(++c_offs == 1024))
					{
						c_offs = 0;
						c_indx++;
						c_page = &current_directory->tables[c_indx]->pages[c_offs];
						c_page_info = &current_directory->tables[c_indx]->pages_info[c_offs];
					}
					if (unlikely(++m_offs == 1024))
					{
						m_offs = 0;
						m_indx++;
						m_page = &kmmap->directory->tables[m_indx]->pages[c_offs];
						m_page_info = &kmmap->directory->tables[m_indx]->pages_info[c_offs];
					}
				}
				
				/*
				 * If the length of this mapping is greater
				 * than the existing mapping we map all the mappings
				 * from the master mapping to our address space, then
				 * map the remaining pages and replace the current
				 * master mapping info with ours.
				 */
				if (mmap->len > kmmap->mmap->len)
				{
					while (c_vaddr < (intptr_t) mmap->addr + mmap->len)
					{
						mmu_alloc_frame(c_vaddr, current_directory, 
							ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
						vfs_read(f->node, f_offset, MMU_PAGE_SIZE, (uint8_t*) c_vaddr);
						c_vaddr += MMU_PAGE_SIZE;
						f_offset += MMU_PAGE_SIZE;
					}
					kmmap->mmap = mmap;
				}
			}
			else	/* if there's no existing mapping... */
			{
				off_t f_offset;
				page_t *c_page;
				page_info_t *c_page_info, *m_page_info;
				uint32_t c_vaddr = (intptr_t) mmap->addr;
				const uint32_t c_addr = c_vaddr / MMU_PAGE_SIZE;
				const uint32_t c_indx = c_addr / 1024;
				const uint32_t c_offs = c_addr % 1024;
				
				kmmap = (kmmap_t*) malloc(sizeof(kmmap_t));
				if (unlikely(kmmap == NULL))
				{
					mutex_release(&kmmaps_lock);
					goto unhandled_fault;
				}
				kmmap->mmap = mmap;
				kmmap->directory = current_directory;
				kmmap->node = f->node;
				kmmap->next = NULL;
				
				assert(current_directory->tables[c_indx] != NULL);
				
				page = &current_directory->tables[c_indx]->pages[c_offs];
				page_info = &current_directory->tables[c_indx]->pages_info[c_offs];
				f_offset = mmap->offset;
				//c_vaddr = (intptr_t) mmap->addr;
				
				while (c_vaddr < (intptr_t) mmap->addr + mmap->len)
				{
					mmu_alloc_frame(c_vaddr, current_directory, 
						ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
					vfs_read(f->node, f_offset, MMU_PAGE_SIZE, (uint8_t*) c_vaddr);
					c_vaddr += MMU_PAGE_SIZE;
					f_offset += MMU_PAGE_SIZE;
				}
					
				/*
				 * Record the mapping on the kernel list
				 */
				pkmmap = &kmmaps;
				while (*pkmmap != NULL)
					pkmmap = &(*pkmmap)->next;
				*pkmmap = kmmap;
			}			

			mutex_release(&kmmaps_lock);
			return;
		}
		/*
		 * The page is mapped but not commited
		 * for write access
		 */
		else if (rw != 0)
		{
			mutex_busywait(&frame_refs_lock);
			if (frame_refs[page->frame] == 1)
			{
				page->rw = 1;
				page_info->commited = 1;
				arch_invalidate_page(address);
				mutex_release(&frame_refs_lock);
				return;
			}
			else
			{
				uint32_t frame;
				frame = page->frame;
				page->frame = 0;
				mmu_alloc_frame(address, current_directory, 
					ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
				assert(page->frame != NULL);
				frame_refs[frame]--;
				
				copy_physical_page(
					page->frame * MMU_PAGE_SIZE, 
					frame * MMU_PAGE_SIZE);
				
				mutex_release(&frame_refs_lock);
				return;
			}
		}
	}
	
unhandled_fault:
	asm ("nop");
	uint32_t esp;
	asm volatile("mov %%esp, %0" : "=r" (esp));
	printk(7, "page_fault: present= %i write= %i user=%i resvd=%i id=%i @ 0x%x",
		p, rw, us, rs, id, addr );
	printk(8, "page_fault: pid: %i tid=%i cs=0x%x esp=%x", getpid(), gettid(), regs->cs, esp);
	printk(8, "page_fault: eip: 0x%x", regs->eip);
	printk(8, "page_fault: eip: 0x%x (%s)", regs->eip, getsym(regs->eip));

	if (getpid() != 0)
	{
		pthread_kill(current_task->id, SIGSEGV);
		schedule();
	}

	mmu_dump_page( addr );
	panic("PAGE_FAULT");
}

/*
 * initialize paging
 */
void mmu_init(multiboot_header_t *p_mboot)
{
	int i;
	uint32_t end_of_kernel;
	uint32_t mem_end_page = 0x10000000;	/* assume 16MB physical memory */

	/*
	 * Enable the a20 line and get the start address
	 * of free memory
	 */
	outb(0x92, inb(0x92) | 0x2); 
	assert(p_mboot->mods_count > 0); /* for now assume there is initrd */
	placement_address = *(uint32_t*) (p_mboot->mods_addr + 4);

	/*
	 * Initialize the frame's reference counters.
	 */
	frame_refs = (uint32_t*) kalloc(sizeof(uint32_t) * (mem_end_page / MMU_PAGE_SIZE),
		NULL, NULL, KALLOC_OPTN_KERNEL | KALLOC_OPTN_NOFREE);
	if (frame_refs == NULL)
		panic("Out of kernel memory!");
	memset(frame_refs, 0, sizeof(uint32_t*) * (mem_end_page / MMU_PAGE_SIZE));
	
	/*
	 * Allocate and initialize the physical memmap
	 */
	if (!memmap_init(&physical_memmap, mem_end_page / MMU_PAGE_SIZE))
		panic("out of memory");

	/*
	 * print memory info
	 */
	printk(7, "mmu_init: kernel image: 0x10000-0x%x", (intptr_t) &end);
	printk(7, "mmu_init: kernel memory: 0x%x", (intptr_t) placement_address);
	printk(7, "mmu_init: free memory: 0x%x-0x%x", (intptr_t) &end, mem_end_page);
	printk(7, "mmu_init: physical pages: %i ", physical_memmap.length);
	printk(7, "mmu_init: total memory: %i MiB", mem_end_page / 1024 / 1024);
    
	/*
	 * initialize the kernel page directory.
	 */
	memset(&kernel_directory, 0, sizeof(page_directory_t));

	/*
	 * Calculate the end of kernel space and make
	 * sure it's on a 4MB boundary.
	 */
	end_of_kernel = placement_address;
	end_of_kernel += MMU_GLOBAL_SPACE;
	if ( end_of_kernel & 0x3FFFFF )
	{
		  end_of_kernel &= 0xFFC00000;
		  end_of_kernel += 0x400000;
	}

	/*
	 * Create virtual memmaps for kernel space and
	 * userspace.
	 */
	if (!memmap_init(&kernel_memmap, end_of_kernel / MMU_PAGE_SIZE))
		panic("out of memory");
	if (!memmap_init(&default_memmap, 1024 * 1024))	/* 4GB */
		panic("out of memory");
	
	if (placement_address & 0xFFF)
	{
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	
	/*
	 * Create all kernel space pages
	 */
	for (i = 0; i < end_of_kernel; i += MMU_PAGE_SIZE)
		mmu_get_page(i, 1, &kernel_directory);

	/*
	 * Map all used kernel pages
	 */
	for (i = 0; i < placement_address; i += MMU_PAGE_SIZE)
	{
		if (unlikely(i >= ucode && i <= data))
		{
			mmu_alloc_frame(i, &kernel_directory, ALLOC_FRAME_USER);			
		}
		else
		{
			mmu_alloc_frame(i, &kernel_directory, 
				ALLOC_FRAME_KERNEL | ALLOC_FRAME_WRITEABLE);
		}
	}

	/*
	 * On the kernel memmap, mark everything that is used
	 * as used. On the user memmap mark the entire kernel space
	 * as used.
	 */
	for (i = 0; i < placement_address; i += MMU_PAGE_SIZE)
	{
		memmap_set_page(&kernel_memmap, i);
		memmap_set_page(&default_memmap, i);
	}
	for (; i < end_of_kernel; i += MMU_PAGE_SIZE)
		memmap_set_page(&default_memmap, i);
	
	/*
	 * Register the page fault handler and enable virtual
	 * memory by switching to a page directory and memmap.
	 */
	register_isr(14, page_fault);
	current_memmap = &default_memmap;
	switch_page_directory(&kernel_directory, 
		(uint32_t) kernel_directory.tablesPhysical);
	
	/*
	 * We need to clone the kernel directory and switch to
	 * the cloned directory. We do this so that when we allocate
	 * the user-mode stack it won't be part of the kernel directory,
	 * so when when we fork() the forked task gets it's own
	 * copy of the stack.
	 */
	current_directory = mmu_clone_directory(
		&kernel_directory, &current_directory_physical, NULL);
	current_memmap = &default_memmap;
	switch_page_directory(current_directory, current_directory_physical);

	printk(8, "mmu_init: placing_address: 0x%x", placement_address);
	printk(8, "mmu_init: %i kernel pages reserved", kernel_memmap.length);
}

/*
 * clones a table entry in a directory.
 * copies physical pages.
 */
static inline page_table_t *mmu_clone_table(
	page_table_t **table,
	page_table_t *src, uint32_t vaddr, uint32_t *physAddr,
	page_directory_t *directory, uint32_t flags)
{
	int i;
	
	/* allocate table in kernel memory */
	*table = (page_table_t*) kalloc(sizeof(page_table_t),
		NULL, physAddr, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);
	if (*table == NULL)
		return NULL;
	memset(*table, 0, sizeof(page_table_t));
	(*table)->noshare = src->noshare;
	
	for (i = 0; i < 1024; i++, vaddr += MMU_PAGE_SIZE)
	{
		if (src->pages[i].frame == NULL)
			continue;

		if ((flags & CLONE_COW) && 
			src->pages_info[i].cow == 1 &&
			src->pages_info[i].noshare == 0)
		{
			src->pages[i].rw = 0;
			src->pages[i].accessed = 0;
			src->pages[i].dirty = 0;
			src->pages_info[i].commited = 0;
			(*table)->pages[i] = src->pages[i];
			(*table)->pages_info[i] = src->pages_info[i];
			frame_refs[(*table)->pages[i].frame]++;
			arch_invalidate_page((*table)->pages[i].frame * MMU_PAGE_SIZE);
		}
		else
		{
			page_t tmp;
		
			/*
			* allocate a physical page
			*/
			mmu_alloc_frame(vaddr, directory,
				ALLOC_FRAME_USER | ALLOC_FRAME_WRITEABLE);
			tmp = src->pages[i];
			tmp.frame = (*table)->pages[i].frame;
			tmp.accessed = 0;
			tmp.dirty = 1;
			(*table)->pages[i] = tmp;
			(*table)->pages_info[i] = src->pages_info[i];
			(*table)->pages_info[i].commited = 1;
			
			copy_physical_page(
				(*table)->pages[i].frame * MMU_PAGE_SIZE, 
				src->pages[i].frame * MMU_PAGE_SIZE);

		}
	}
	return *table;
}

/*
 * clones a page directory.
 *
 */
page_directory_t *mmu_clone_directory(
	page_directory_t *src, intptr_t *phys, uint32_t flags)
{
	int i;
	uint32_t physmem;
	page_directory_t *dir;
	
	/* allocate page directory in kernel memory */
	dir = (page_directory_t*) kalloc(sizeof(page_directory_t),
		NULL, &physmem, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);
	if (unlikely(dir == NULL))
		return NULL;
	memset(dir, 0, sizeof(page_directory_t));

	if (phys != NULL)
		*phys = physmem + ((uint32_t) dir->tablesPhysical - (uint32_t) dir);
	
	for (i = 0; i < 1024; i++)
	{
		if (!src->tables[i])
			continue;

		/* To determine if a page belongs to kernel or user-space we
		 * simply compare it to the kernel directory. If it is thenn we 
		 * link to the kernel's page entry, otherwise we create a new
		 * page and copy the contents
		 */
		if ((flags & CLONE_SHARE && src->tables[i]->noshare == 0) || 
			kernel_directory.tables[i] == src->tables[i])
		{
			dir->tables[i] = src->tables[i];
			dir->tablesPhysical[i] = src->tablesPhysical[i];
			dir->tables[i]->refs++;
		}
		else
		{
			physmem = 0;
			dir->tables[i] = mmu_clone_table(
				&dir->tables[i],
				src->tables[i], 
				i * 1024 * MMU_PAGE_SIZE,
				&physmem, dir, flags);
			if (dir->tables[i] == NULL)
			{
				mmu_destroy_directory(dir);
				return NULL;
			}
			dir->tablesPhysical[i] = physmem | 0x07;
		}
	}

	return dir;
}

/*
 * Free all memory used by page directory.
 */
void mmu_destroy_directory(page_directory_t *directory)
{
	int i, j;
	
	assert(directory != NULL);
	
	for (i = 0; i < 1024; i++)
	{
		page_table_t *table;
		table = directory->tables[i];
		if (table == NULL)
			continue;
		
		if (kernel_directory.tables[i] != table)
		{
			for (j = 0; j < 1024; j++)
			{
				page_info_t *page_info;
				page_info = &table->pages_info[j];
				if (table->pages[j].frame != NULL)
					mmu_frame_free(&table->pages[j]);
			}
			if (--directory->tables[i]->refs == 0)
				kfree((void*) table);
		}
		else
		{
			kernel_directory.tables[i]->refs--;
		}
	}
	kfree((void*) directory);
}

void mmu_purge_directory(page_directory_t *directory)
{
	assert(0);
}

/*
 * 
 */
int brk(void *addr)
{
	return ENOSYS;
}

int sbrk(intptr_t inc)
{
	return ENOSYS;
}
