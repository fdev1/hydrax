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
	/*
	 * Even though 0 is a valid physical frame
	 * address it is safe to say that frame 0 will
	 * never be freed. Right now it is allocated and 
	 * not used but we need to allocate it to the heap
	 * at some point.
	 */
	if (page->frame == 0)
		return;

	mutex_wait(&frame_refs_lock);
	if (--frame_refs[page->frame] == 0)
	{
		mutex_wait(&physical_memmap.lock);
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
static uint32_t mmu_alloc_frame(uint32_t address, page_directory_t *directory)
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
		
		mutex_wait(&physical_memmap.lock);
		idx = memmap_find_free_page(&physical_memmap);
		if (idx == (uint32_t) -1)
			panic("Out of physical memory!");
		memmap_set_page(&physical_memmap, idx * MMU_PAGE_SIZE);
		mutex_release(&physical_memmap.lock);
		
		page->present = 1;
		page->accessed = 0;
		page->dirty = 0;
		page->frame = idx;
		page_info->commited = 1;
		
		/*
		 * FIXME: At some point this is being called with
		 * the frame_refs_lock mutex held.
		 */
		/* mutex_wait(&frame_refs_lock); */
		frame_refs[idx]++;
		/* mutex_release(&frame_refs_lock); */
		
		return idx * MMU_PAGE_SIZE;
	}
}

/*
 * Allocate a page of virtual memory to the specified
 * directory.
 */
page_t *mmu_alloc_page(page_directory_t *dir, 
	intptr_t vaddr, unsigned int flags, uint32_t *phys)
{
	page_t *page;
	page_info_t *page_info;
	
	const uint32_t address = vaddr / MMU_PAGE_SIZE;
	const uint32_t table_idx = address / 1024;
	const uint32_t page_idx = address % 1024;
	
	/*
	 * if the page table does not exist and the MKTABLE option
	 * was specified then create a table
	 */
	if (unlikely(dir->tables[table_idx] == NULL))
	{
		if (likely((flags & ALLOC_FRAME_MKTABLE) != 0))
		{
			uint32_t tmp;
			dir->tables[table_idx] = (page_table_t*) kalloc(sizeof(page_table_t), 
				NULL, &tmp, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);
			if (unlikely(dir->tables[table_idx] == NULL))
			{
				if (current_task != NULL)
					current_task->errno = ENOMEM;
				return NULL;
			}
			memset(dir->tables[table_idx], 0, sizeof(page_table_t));
			dir->tablesPhysical[table_idx] = tmp | 0x7; /* PRESENT, RW, US.*/
		}
		else
		{
			return NULL;
		}
	}

	page = &dir->tables[table_idx]->pages[page_idx];
	page_info = &dir->tables[table_idx]->pages_info[page_idx];
	page->rw = (flags & ALLOC_FRAME_WRITEABLE) ? 1 : 0;
	page->user = (flags & ALLOC_FRAME_KERNEL) ? 0 : 1;
	page_info->cow = ((flags & ALLOC_FRAME_COW) != 0);
	page_info->noshare = ((flags & ALLOC_FRAME_NOSHARE) != 0);
	page_info->mmap = ((flags & ALLOC_FRAME_MMAP) != 0);
	
	/*
	 * If a page is private then it's table needs to be private too,
	 * otherwise the clone function will assume the entire table is
	 * shared without looking at the pages.
	 */
	if (unlikely(page_info->noshare == 1 && dir->tables[table_idx]->noshare != 1))
		dir->tables[table_idx]->noshare = 1;
	
	if (unlikely((flags & ALLOC_FRAME_ALLOC_PHYS) != 0))
	{
		uint32_t phmem;
		if (unlikely(phys == NULL))
			phys = &phmem;
		*phys = mmu_alloc_frame(vaddr, dir);
	}
	
	return page;
}

/*
 * gets a page structure from the specified directory
 */
static inline page_t *mmu_get_page(page_directory_t* dir, uint32_t address)
{
	address /= MMU_PAGE_SIZE;
	const uint32_t table_idx = address / 1024;
	const uint32_t page_idx = address % 1024;

	if (unlikely(dir->tables[table_idx] == NULL))
		return NULL;
	return &dir->tables[table_idx]->pages[page_idx];
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
 * NOTE: If vaddr is not NULL it must be aligned on a page boundary!
 * 
 * 
 * TODO: This needs to be cleaned up soon as it's getting messy.
 */
intptr_t kalloc(uint32_t sz, uint32_t vaddr, uint32_t *phys, uint32_t flags)
{
	page_t *page;
	page_directory_t *dir;
	memmap_t *memmap;
	intptr_t tmp_vaddr;
	buffer_t **buffers;
	uint32_t physmem;
	unsigned int pflags;
	
	pflags = 0;
	
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
		pflags |= ALLOC_FRAME_KERNEL;
	}
	else
	{
		memmap = current_memmap;
		dir = current_directory;
		buffers = &current_task->buffers;
		pflags |= ALLOC_FRAME_USER;
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

	
	const uint32_t cow = (flags & KALLOC_OPTN_NOCOW) ? 0 : 1;
	
	pflags = ALLOC_FRAME_MKTABLE;

	if (cow)
		pflags |= ALLOC_FRAME_COW;
	if ((flags & KALLOC_OPTN_NOSHARE) != 0)
		pflags |= ALLOC_FRAME_NOSHARE;
	if ((flags & KALLOC_OPTN_MMAP) != 0)
		pflags |= ALLOC_FRAME_MMAP;
	else pflags |= ALLOC_FRAME_ALLOC_PHYS;
	
	/*
	 * TODO: We need to make sure that anyone kalloc'ing
	 * anything that needs write access passes the 
	 * KALLOC_OPTN_WRITEABLE! For now we just make
	 * everything writeable.
	 */
	/* if ((flags & KALLOC_OPTN_WRITEABLE) != 0) */
		pflags |= ALLOC_FRAME_WRITEABLE;
		
	/*
	 * Allocate and map all pages requested.
	 */
	page = mmu_alloc_page(dir, vaddr, pflags, phys);
	assert(page != NULL);	
	for (tmp_vaddr = vaddr + 0x1000; tmp_vaddr < vaddr + sz; tmp_vaddr += 0x1000)
	{
		page = mmu_alloc_page(dir, tmp_vaddr, pflags, NULL);
		assert(page != NULL);
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

	assert(*tmp != NULL);

	addr = (intptr_t) (*tmp)->address;
	for (i = 0; i < (*tmp)->pages; i++)
	{
		mmu_frame_free(mmu_get_page(dir, addr));
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

/*
 * Switches to the kernel directory. This is
 * used by exit() to switch to the kernel directory
 * before destroying the current directory.
 * It may be cleaner, to make mmu_destroy_directory() 
 * not take a directory argument (ie. just destroy the 
 * current directory).
 */
void mmu_switch_to_kernel_directory(void)
{
	current_memmap = &default_memmap;
	switch_page_directory(&kernel_directory, 
		(uint32_t) kernel_directory.tablesPhysical);
}

/*
 * Make sure that the page range associated with the
 * destination address has all the mappings of the source
 * range.
 */
static void mmap_dup(page_directory_t *dir, intptr_t src, intptr_t dst, size_t len)
{
	page_t *s_page, *d_page;
	page_info_t *s_page_info, *d_page_info;
	uint32_t s_addr, s_indx, s_offs, d_addr, d_indx, d_offs;
	
	s_addr = src / MMU_PAGE_SIZE;
	d_addr = dst / MMU_PAGE_SIZE;
	s_indx = s_addr / 1024;
	d_indx = d_addr / 1024;
	s_offs = s_addr % 1024;
	d_offs = d_addr % 1024;

	s_page = &dir->tables[s_indx]->pages[s_offs];
	s_page_info = &dir->tables[s_indx]->pages_info[s_offs];
	d_page = &current_directory->tables[d_indx]->pages[d_offs];
	d_page_info = &current_directory->tables[d_indx]->pages_info[d_offs];

	while (len > 0)
	{
		if (s_page->present == 1 && d_page->present == 0)
		{
			mutex_wait(&frame_refs_lock);
			frame_refs[s_page->frame]++;
			mutex_release(&frame_refs_lock);
			
			if (s_page->rw == 1)
			{
				s_page->rw = 0;
				s_page_info->commited = 0;
			}
			
			*d_page = *s_page;
			*d_page_info = *s_page_info;	
			arch_invalidate_page(dst);
		}
		
		s_page++;
		d_page++;
		s_page_info++;
		d_page_info++;
		s_indx++;
		d_indx++;
		s_offs++;
		d_offs++;
		
		if (unlikely(s_offs == 1024))
		{
			s_indx++;
			s_offs = 0;
			s_page = &dir->tables[s_indx]->pages[s_offs];
			s_page_info = &dir->tables[s_indx]->pages_info[s_offs];
		}
		if (unlikely(d_offs == 1024))
		{
			d_indx++;
			s_offs = 0;
			d_page = &current_directory->tables[d_indx]->pages[d_offs];
			d_page_info = &current_directory->tables[d_indx]->pages_info[d_offs];
		}
		
		src += MMU_PAGE_SIZE;
		dst += MMU_PAGE_SIZE;
		len -= MMU_PAGE_SIZE;
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

			/*
			 * Search a mmap that covers the current address
			 * on the current task mmaps.
			 * 
			 * NOTE: Right now we just handle the fault as
			 * "unhandled" (ie. kill the faulting process) but
			 * we can probably just panic since a missing mmap
			 * would indicate a bug.
			 */
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
				page_t *m_page;
				page_info_t *m_page_info;

				/*
				 * Calculate the address and location of the page
				 * structure for the master mapping.
				 */
				const intptr_t m_address = 
					(((intptr_t) kmmap->mmap->addr) + (address - (intptr_t) mmap->addr)) &
					~0xFFF;
				const intptr_t m_addr = m_address / MMU_PAGE_SIZE;
				const uint32_t m_indx = m_addr / 1024;
				const uint32_t m_offs = m_addr % 1024;
				
				/*
				 * Get the page and page_info structure for the
				 * master mapping
				 */
				m_page = &kmmap->directory->tables[m_indx]->pages[m_offs];
				m_page_info = &kmmap->directory->tables[m_indx]->pages_info[m_offs];
				
				/*
				 * If the page has not been mapped on the master
				 * mapping then map it.
				 */
				if (unlikely(m_page->present == 0))
				{
					address &= ~0xFFF;
					mmu_alloc_frame(address, kmmap->directory);
					vfs_read(f->node, mmap->offset + (address - (intptr_t) mmap->addr), 
						MMU_PAGE_SIZE, (uint8_t*) address);
					/* TODO: Mark the page as clean */
				}
				
				/*
				 * If the current page is not the master page
				 * we set the current page to reference the master
				 * page
				 */
				if (m_page != page)
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
					
					*page = *m_page;
					*page_info = *m_page_info;	
					arch_invalidate_page(address);
					
					/*
					 * If the length of this mapping is greater
					 * than the existing mapping we replace the current
					 * master mapping info with ours.
					 */
					if (unlikely(mmap->len > kmmap->mmap->len))
					{
						mmap_dup(kmmap->directory, (intptr_t) kmmap->mmap->addr,
							(intptr_t) mmap->addr, kmmap->mmap->len);
						/* TODO: This needs to be done atomically */
						kmmap->mmap = mmap;
						kmmap->directory = current_directory;
					}
				}
			}
			else	/* if there's no existing mapping... */
			{
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
				
				address &= ~0xFFF;
				mmu_alloc_frame(address, current_directory);
				vfs_read(f->node, mmap->offset + (address - (intptr_t) mmap->addr), 
					    MMU_PAGE_SIZE, (uint8_t*) address);
				arch_invalidate_page(address);
					
				/*
				 * Record the mapping on the kernel list
				 */
				pkmmap = &kmmaps;
				while (*pkmmap != NULL)
					pkmmap = &(*pkmmap)->next;
				*pkmmap = kmmap;
			}			

			mutex_release(&kmmaps_lock);
			if (rw == 0)
				return;
		}
		/*
		 * The page is mapped but not commited
		 * for write access
		 * 
		 * FIXME: We need to release frame_refs_lock
		 * before we call mmu_alloc_frame().
		 */
		if (rw != 0)
		{
			mutex_wait(&frame_refs_lock);
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
				mmu_alloc_frame(address, current_directory);
				assert(page->frame != NULL);
				page->rw = 1;
				frame_refs[frame]--;
				
				copy_physical_page(
					page->frame * MMU_PAGE_SIZE, 
					frame * MMU_PAGE_SIZE);
				
				mutex_release(&frame_refs_lock);
				return;
			}
		}
		return;
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

	mmu_dump_page(addr);
	panic("Page Fault!");
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
	{
		if (unlikely(i >= ucode && i <= data))
		{
			mmu_alloc_page(&kernel_directory, i, 
				ALLOC_FRAME_MKTABLE | ALLOC_FRAME_USER, NULL);
		}
		else
		{
			mmu_alloc_page(&kernel_directory, i, 
				ALLOC_FRAME_MKTABLE | ALLOC_FRAME_KERNEL | ALLOC_FRAME_WRITEABLE, NULL);
		}
	}

	/*
	 * Map all used kernel pages
	 */
	for (i = 0; i < placement_address; i += MMU_PAGE_SIZE)
		mmu_alloc_frame(i, &kernel_directory);			

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
	*table = (page_table_t*) kalloc(sizeof(page_table_t), NULL, physAddr, 
		KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL | KALLOC_OPTN_WRITEABLE);
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
			/*
			 * TODO: don't double copy
			 */
			page_t tmp;
		
			/*
			* allocate a physical page
			*/
			mmu_alloc_frame(vaddr, directory);
			tmp = src->pages[i];
			tmp.frame = (*table)->pages[i].frame;
			tmp.accessed = 0;
			tmp.dirty = 0;
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
 */
page_directory_t *mmu_clone_directory(
	page_directory_t *src, intptr_t *phys, uint32_t flags)
{
	int i;
	uint32_t physmem;
	page_directory_t *dir;
	
	/* allocate page directory in kernel memory */
	dir = (page_directory_t*) kalloc(sizeof(page_directory_t), NULL, &physmem, 
		KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL | KALLOC_OPTN_WRITEABLE);
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
