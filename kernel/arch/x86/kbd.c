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
#include <errno.h>
#include <semaphore.h>
#include "../../config.inc"

#if defined(KERNEL_CODE)
#	undef KERNEL_CODE
#endif
#define KERNEL_CODE

#define KBD_LEFT_SHIFT		(128)
#define KBD_RIGHT_SHIFT		(129)
#define KBD_ALT			(130)

static vfs_node_t *kbd = NULL;
static keymask_t keymask = { 0 };
static unsigned char *keymap;

static unsigned char *kbd_input_buffer = NULL;
static unsigned char *kbd_input_buffer_rhead = NULL;
static unsigned char *kbd_input_buffer_whead = NULL;
static unsigned int kbd_input_buffer_len = 0;
static semaphore_t kbd_semaphore;


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
				
				if (kbd_input_buffer_len < CONFIG_KBD_INPUT_BUFFER_SIZE)
				{
					*kbd_input_buffer_whead++ = c;
					if (kbd_input_buffer_whead >=
						&kbd_input_buffer[CONFIG_KBD_INPUT_BUFFER_SIZE])
						kbd_input_buffer_whead = kbd_input_buffer;
					kbd_input_buffer_len++;
					semaphore_signal_sleep(&kbd_semaphore);
				}
				break;
		}
	}
}

/*
 * Read from the keyboard buffer.
 * This function blocks if the buffer is empty.
 */
static int kbd_read(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	size_t bytes_read = 0;
	while (len > 0)
	{
		semaphore_waitsleep(&kbd_semaphore);
		*buf++ = *kbd_input_buffer_rhead++;
		if (kbd_input_buffer_rhead >=
			&kbd_input_buffer[CONFIG_KBD_INPUT_BUFFER_SIZE])
			kbd_input_buffer_rhead = kbd_input_buffer;
		len--;
		kbd_input_buffer_len--;
		bytes_read++;
	};
	return bytes_read;
}

static int kbd_ioctl(vfs_node_t *node, unsigned int request, void *last_arg)
{
	return ENOSYS;
}

/*
 *
 */
void kbd_init(void)
{
	int r;
	kbd = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (kbd == NULL)
		panic("kbd: out of memory!");
	*kbd = vfs_node_init(FS_CHARDEVICE);
	strcpy(kbd->name, "kbd");
	kbd->read = (vfs_read_fn_t) &kbd_read;
	kbd->ioctl = (vfs_ioctl_fn_t) &kbd_ioctl;
	r = devfs_mknod(kbd);
	if (r == -1)
		panic("kbd: could not create dev node!");
	keymap = kbdus;
	
	/*
	 * allocate input buffer and register keyboard handler
	 */
	kbd_input_buffer = (unsigned char*) malloc(CONFIG_KBD_INPUT_BUFFER_SIZE);
	if (kbd_input_buffer == NULL)
		panic("tty_init: out of memory!");
	kbd_semaphore = SEMAPHORE_INITIALIZER(
		CONFIG_KBD_INPUT_BUFFER_SIZE, CONFIG_KBD_INPUT_BUFFER_SIZE);
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
