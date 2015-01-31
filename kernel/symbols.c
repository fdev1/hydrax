/*
 * symbols.c
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
#include <memory.h>
#include <vfs.h>
#include <printk.h>

typedef struct
{
	uint32_t address;
	char name[100];
}
symbol_t;

symbol_t symbols[1000];

void symbols_init(void)
{
	uint32_t i = 0;
	vfs_node_t *node = vfs_open("/lib/hydrax.syms", NULL);
	if (likely(node != NULL))
	{
		memset(symbols, 0, sizeof(symbol_t) * 1000);
		vfs_read(node, 0, 1000 * sizeof(symbol_t), (unsigned char*) symbols);
		printk(8, "symbols: loaded");
	}

	printk(7, "symbols_init: symbols ready");
}

char *getsym(uint32_t addr)
{
	symbol_t *cur_sym = NULL;
	unsigned int i;

	for (i = 0; i < 1000; i++)
	{
		if (symbols[i].address <= addr)
		{
			if (cur_sym == NULL || symbols[i].address > cur_sym->address)
				cur_sym = &symbols[i];
		}														
	}
	
	if (cur_sym != NULL && cur_sym->address >= 0x8000000)
		return "<user-space>";
	
	return (cur_sym != NULL) ? cur_sym->name : NULL;
}

