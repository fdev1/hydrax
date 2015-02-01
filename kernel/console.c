/*
 * console.c
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
#include <arch/video.h>
#include <arch/kbd.h>
#include <printk.h>
#include <vfs.h>
#include <kheap.h>
#include <string.h>
#include <devfs.h>

#define MEM_CHAR(c)	(c | (attributeByte << 8))

vfs_node_t console;

static vfs_node_t *kbd;
static bool early_init = false;

/*
 * Read from the console.
 * This just reads from the keyboard driver.
 */
static uint32_t console_read(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	return vfs_read(kbd, offset, len, buf);
}

/*
 * Write to the console.
 * This is where we'll be implementing all the 
 * console escape codes.
 */
static uint32_t console_write(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	int i;
	i = 0;
	while (len--)
	{
		arch_video_put(*buf++);
		i++;
	}
	return i;
}

/*
 * Initialize video driver and the console.
 */
void console_early_init(void)
{
	arch_video_init();
	early_init = true;
	console = VFS_NODE_INITIALIZER(FS_CHARDEVICE);
	strcpy(console.name, "console");
	console.write = (vfs_write_fn_t) &console_write;
}

/*
 * Initialize the console.
 */
void console_init(void)
{
	int r;
	if (!early_init)
		console_early_init();
	
	kbd_init();
	kbd = vfs_open("/dev/kbd", NULL);
	if (kbd == NULL)
		panic("console: cannot open /dev/kbd!");
	console.read = (vfs_read_fn_t) &console_read;
	
	r = devfs_mknod(&console);
	if (r == -1)
		panic("console: could not create dev node!");
	printk(7, "console: console ready");
	return;	
}
