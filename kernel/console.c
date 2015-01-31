/*
 * This file is part of Project Athena.
 * Copyright (c) 2014 Fernando Rodriguez.
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

vfs_node_t *console;

static void console_kbd_handler(char key, keymask_t mask)
{
	char s[2] = {0,0};
	s[0] = key;
	kprintf("%s", s);
}

/*
 * Initialize video
 */
void console_early_init(void)
{
	arch_video_init();
}

void console_init(void)
{
	int r;
	
	kbd_init();
	
	console = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (console == NULL)
		panic("console_init: out of memory!");
	*console = vfs_node_init(FS_CHARDEVICE);
	strcpy(console->name, "console");
	console->read = NULL;
	console->write = NULL;

	r = devfs_mknod(console);
	if (r == -1)
		panic("console_init: could not create dev node!");
	printk(7, "console_init: console ready");
	return;	
}

void video_put(char c)
{
	arch_video_put(c);
}

void video_clear(void)
{
	arch_video_clear();
}

/*
 * Put a null-terminated string on display.
 */
void video_put_s(const char *s)
{
	arch_video_put_s(s);
}


