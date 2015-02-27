/*
 * include/memmap.h
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

#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include <stdint.h>
#include <stdbool.h>
#include <mutex.h>

#define memmap_page_index(a) (a / (CHAR_BIT * sizeof(uint32_t)))
#define memmap_bytes(memmap)	(memmap->length * sizeof(uint32_t))


typedef struct 
{
	uint32_t *pagemap;
	uint32_t length;
	mutex_t lock;
}
memmap_t;

/*
 * Initialize a memmap.
 */
bool memmap_init(memmap_t *map, uint32_t pages);

/*
 * Clone a memmap
 */
bool memmap_clone(memmap_t *dest, memmap_t *src);

/*
 * Free all memory used by memmap.
 */
void memmap_destroy(memmap_t *map);

/*
 * Marks a page as used in the memmap.
 */
void memmap_set_page(memmap_t *map, uint32_t addr);

/*
 * Mark a page as free in the memmap.
 */
void memmap_clear_page(memmap_t *map, uint32_t page_addr);

/*
 * Test a page in the memmap.
 */
uint32_t memmap_test_page(memmap_t *map, uint32_t page_addr);

/*
 * Find the first free page on the memmap.
 */
uint32_t memmap_find_free_page(memmap_t *map);

/*
 * Find consecutive pages.
 */
uint32_t memmap_find_free_pages(memmap_t *map, uint32_t pages);


#endif
