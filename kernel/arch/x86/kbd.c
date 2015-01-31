/*
 * kbd.c
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
#include <arch/kbd.h>
#include <printk.h>
#include <isr.h>
#include <kheap.h>
#include <vfs.h>
#include <devfs.h>
#include <string.h>

#if defined(KERNEL_CODE)
#undef KERNEL_CODE
#endif
#define KERNEL_CODE

#define KBD_LEFT_SHIFT		(128)
#define KBD_RIGHT_SHIFT		(129)
#define KBD_ALT			(130)

typedef struct kbd_handler_list
{
	kbd_handler_t handler;
	struct kbd_handler_list *next;
	
}
kbd_handler_list_t;

static keymask_t keymask = { 0 };
static kbd_handler_list_t *handlers = NULL;
static unsigned char *keymap;

static unsigned char kbdus[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 
	'\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
  	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
 	0,			/* 29   - Control */
 	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 	'\'', '`',   
 	KBD_LEFT_SHIFT,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   
	KBD_RIGHT_SHIFT,	/* Right shift */
  	'*',
	KBD_ALT,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};	

/* 
 * keboard input handler
 */
static void kbd_input_handler(registers_t *rigs)
{
	uint8_t scan_code;
	unsigned char c;
	scan_code = inb(0x60);

	if ((scan_code & 0x80))
	{
		c = keymap[scan_code & 0x7F];
		switch (c)
		{
			case KBD_LEFT_SHIFT:
				keymask.left_shift = 0;
				break;
			case KBD_RIGHT_SHIFT:
				keymask.right_shift = 0;
				break;
			case KBD_ALT:
				keymask.alt = 0;
				break;
		}
	}
	else
	{

		c = keymap[scan_code];

		switch (c)
		{
			case KBD_LEFT_SHIFT:
				keymask.left_shift = 1;
				break;
			case KBD_RIGHT_SHIFT:
				keymask.right_shift = 1;
				break;
			case KBD_ALT:
				keymask.alt = 1;
			default:

				if (keymask.left_shift || keymask.right_shift)
				{
					if (c >= 97 && c <= 122)
						c -= (97 - 65);

				}

				kbd_handler_list_t *tmp;
				tmp = handlers;
				while (tmp != NULL)
				{
					tmp->handler(c, keymask);
					tmp = tmp->next;
				}
				break;
		}
	}
}

/*
 *
 */
void kbd_init(void)
{
	int r;
	vfs_node_t *kbd;
	kbd = (vfs_node_t*) malloc(sizeof(vfs_node_t)); /* we dont need to track this */
	if (kbd == NULL)
		panic("kbd: out of memory!");
	*kbd = vfs_node_init(FS_CHARDEVICE);
	strcpy(kbd->name, "kbd");
	kbd->read = NULL;
	kbd->write = NULL;

	r = devfs_mknod(kbd);
	if (r == -1)
		panic("kbd: could not create dev node!");
	keymap = kbdus;
	register_isr(IRQ1, &kbd_input_handler);
	printk(7, "kbd: keyboard ready");
}

/*
 * sets the keymap
 */
void kbd_setkeymap(unsigned char *map)
{
	keymap = map;
}

/*
 * registers a handler for kbd input
 */
void* kbd_register_handler(kbd_handler_t handler)
{
	kbd_handler_list_t *h, *tmp;
	
	/* TODO: why are we not using malloc()? */
	h = (kbd_handler_list_t*) malloc(sizeof(kbd_handler_list_t));
	assert(h != NULL);

	h->handler = handler;
	h->next = NULL;

	if (handlers == NULL)
	{
		handlers = h;
	}
	else
	{
		tmp = handlers;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = h;
	}
	return (void*) h;
}



