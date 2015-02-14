#include <mmu.h>
#include <printk.h>

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

