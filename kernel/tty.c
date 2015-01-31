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
#include <console.h>
#include <kbd.h>
#include <printk.h>
#include <vfs.h>
#include <devfs.h>
#include <string.h>
#include <kheap.h>
#include <devfs.h>
#include <semaphore.h>

#define TTY_INPUT_BUFFER_SIZE		(1024)

/*
 * FIXME: The way where handling locking here
 * can deadlock the kernel!
 */

/*
 * static members
 */
static unsigned char *tty_input_buffer = NULL;
static unsigned char *tty_input_buffer_rhead = NULL;
static unsigned char *tty_input_buffer_whead = NULL;
static unsigned int tty_input_buffer_len = 0;
static semaphore_t tty_semaphore;

/*
 * read from the virtual console (keyboard)
 */
/*static*/ uint32_t tty_read(vfs_node_t *node, 
	uint32_t offset, uint32_t length, uint8_t *ptr)
{
	size_t bytes_read = 0;
	while (length > 0)
	{
		semaphore_waitsleep(&tty_semaphore);
		*ptr++ = *tty_input_buffer_rhead++;
		if (tty_input_buffer_rhead >=
			&tty_input_buffer[TTY_INPUT_BUFFER_SIZE])
			tty_input_buffer_rhead = tty_input_buffer;
		length--;
		tty_input_buffer_len--;
		bytes_read++;
	};
	return bytes_read;
}

/*
 * writes to the virtual console (screen)
 */
/*static*/ uint32_t tty_write(vfs_node_t *node, uint32_t offset, unsigned int count, uint8_t *ptr)
{
	unsigned int i;
	for (i = 0; i < count; i++)
		video_put(*ptr++);
	return i;
}

/*
 * keyboard interrupt handler
 */
/*static*/ void tty_kbd_handler(char key, keymask_t mask)
{
	if (tty_input_buffer_len < TTY_INPUT_BUFFER_SIZE)
	{
		*tty_input_buffer_whead++ = key;
		if (tty_input_buffer_whead >=
			&tty_input_buffer[TTY_INPUT_BUFFER_SIZE])
			tty_input_buffer_whead = tty_input_buffer;
		tty_input_buffer_len++;
		semaphore_signal_sleep(&tty_semaphore);
	}
}


/*
 * Initialize video
 */
void tty_init(void)
{
	int r;
	vfs_node_t *tty;
	tty = (vfs_node_t*) malloc(sizeof(vfs_node_t));	/* we dont need to track this */
	if (tty == NULL)
		panic("tty_init: out of memory!");
	*tty = vfs_node_init(FS_CHARDEVICE);
	strcpy(tty->name, "tty");
	tty->major = 1;
	tty->minor = 1;
	tty->read = (read_type_t) &tty_read;
	tty->write = (write_type_t) &tty_write;
	tty->length = sizeof(vfs_node_t);

	r = devfs_mknod(tty);
	if (r == -1)
		panic("tty_init: could not create dev node!");

	/*
	 * allocate input buffer and register keyboard handler
	 */
	tty_input_buffer = (unsigned char*) malloc(TTY_INPUT_BUFFER_SIZE);
	if (tty_input_buffer == NULL)
		panic("tty_init: out of memory!");
	tty_semaphore = SEMAPHORE_INITIALIZER(TTY_INPUT_BUFFER_SIZE, TTY_INPUT_BUFFER_SIZE);
	kbd_register_handler(&tty_kbd_handler);

	assert(vfs_open("/dev/tty", NULL) == tty);
	printk(7, "tty_init: tty ready");
	return;
}



