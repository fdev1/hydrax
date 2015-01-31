/*
 * tty.c
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

#include <arch/arch.h>
#include <arch/kbd.h>
#include <console.h>
#include <printk.h>
#include <vfs.h>
#include <devfs.h>
#include <string.h>
#include <kheap.h>
#include <devfs.h>

/*
 * static members
 */
static vfs_node_t *console;

/*
 * read from the virtual console (keyboard)
 */
static uint32_t tty_read(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	return vfs_read(console, offset, len, buf);
}

/*
 * writes to the virtual console (screen)
 */
static uint32_t tty_write(vfs_node_t *node, uint32_t offset, unsigned int len, uint8_t *buf)
{
	return vfs_write(console, offset, len, buf);
}

/*
 * Initialize video
 */
void tty_init(void)
{
	int r;
	vfs_node_t *tty;
	tty = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (tty == NULL)
		panic("tty_init: out of memory!");
	*tty = vfs_node_init(FS_CHARDEVICE);
	strcpy(tty->name, "tty");
	tty->major = 1;
	tty->minor = 1;
	tty->read = (vfs_read_fn_t) &tty_read;
	tty->write = (vfs_write_fn_t) &tty_write;
	tty->length = sizeof(vfs_node_t);

	r = devfs_mknod(tty);
	if (r == -1)
		panic("tty_init: could not create dev node!");

	/*
	 * Open the console device
	 */
	console = vfs_open("/dev/console", NULL);
	if (console == NULL)
		panic("tty: could not open /dev/console!");
	printk(7, "tty: tty ready");
	return;
}
