/*
 * memmap.c
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

#include <arch/platform.h>
#include <memmap.h>
#include <memory.h>
#include <mmu.h>

#define page_index(a)		(memmap_page_index(a))
#define OFFSET_FROM_BIT(a) 	(a % (CHAR_BIT * sizeof(uint32_t)))

/*
 * Initialize a memmap
 */
bool memmap_init(memmap_t *map, uint32_t pages)
{
	map->pagemap = (uint32_t*) kalloc(
		page_index(pages) * sizeof(uint32_t), 
		NULL, NULL, KALLOC_OPTN_KERNEL);
	if (map->pagemap == NULL)
		return false;
	memset(map->pagemap, 0, 
		page_index(pages) * sizeof(uint32_t));
	map->lock = MUTEX_INITIALIZER;
	map->length = pages;
	return true;
}

/*
 * Clone a memmap
 */
bool memmap_clone(memmap_t *dest, memmap_t *src)
{
	dest->pagemap = (uint32_t*) kalloc(
		page_index(src->length) * sizeof(uint32_t), 
		NULL, NULL, KALLOC_OPTN_KERNEL);
	if (dest->pagemap == NULL)
		return false;
	memcpy(dest->pagemap, src->pagemap, 
		page_index(src->length) * sizeof(uint32_t));
	dest->lock = MUTEX_INITIALIZER;
	dest->length = src->length;
	return true;
}

/*
 * Free all memory used by memmap.
 */
void memmap_destroy(memmap_t *map)
{
	if (map == NULL)
		return;
	kfree(map->pagemap);
	map->pagemap = NULL;
	map->length = 0;
}

/*
 * Marks a page as used.
 */
void memmap_set_page(memmap_t *map, uint32_t page_addr)
{
	assert((page_addr & 0xFFF) == 0);
	page_addr /= 0x1000;
	map->pagemap[page_index(page_addr)] |= 
		(1 << OFFSET_FROM_BIT(page_addr));
}

/*
 * Mark a page as free in the memmap.
 */
void memmap_clear_page(memmap_t *map, uint32_t page_addr)
{
	uint32_t frame = page_addr / 0x1000;
	uint32_t index = page_index(frame);
	uint32_t offset = OFFSET_FROM_BIT(frame);
	map->pagemap[index] &= ~(1 << offset);
}

/*
 * Test a page in the memmap.
 */
uint32_t memmap_test_page(memmap_t *map, uint32_t page_addr)
{
	uint32_t frame, index, offset;
	frame = page_addr / 0x1000;
	index = page_index(frame);
	offset = OFFSET_FROM_BIT(frame);
	return (map->pagemap[index] & (1 << offset)) ? 1 : 0;
}

/*
 * Find the next free page on the memmap.
 */
uint32_t memmap_find_free_page(memmap_t *map)
{
	uint32_t i, j;
	for (i = 0; i < page_index(map->length); i++)
		if (map->pagemap[i] != 0xFFFFFFFF) 
			for (j = 0; j < 32; j++)
				if ( !(map->pagemap[i] & (1 << j)) )
					return i*4*8+j;
	return NULL;
}

/*
 * Find consecutive free pages.
 */
uint32_t memmap_find_free_pages(memmap_t *map, uint32_t pages)
{
	uint32_t i, j;
	uint32_t start_page, found;
	assert(pages != 0);
	found = 0;
	for (i = 0; i < page_index(map->length); i++)
	{
		if (map->pagemap[i] != 0xFFFFFFFF) 
		{
			for (j = 0; j < 32; j++)
			{
				if (!(map->pagemap[i] & (1 << j)))
				{
					if (found++ == 0)
						start_page = i * 4 * 8 + j;
					if (found == pages)
						return start_page;
				}
				else
				{
					found = 0;
				}
			}
		}
		else
		{
			found = 0;
		}
	}
	return NULL;
}
