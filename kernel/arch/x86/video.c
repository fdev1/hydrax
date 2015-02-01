/*
 * vga.c
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
#include <vfs.h>
#include <kheap.h>
#include <string.h>

#define MEM_CHAR(c)	(c | (attributeByte << 8))

static uint8_t cur_x = 0;
static uint8_t cur_y = 0;
static uint16_t *vmem = (uint16_t*) 0xB8000;
static const uint8_t attributeByte = (0 /*black*/ << 4) | (7 /*white*/ & 0x0F);
static vfs_node_t video;
/*
 * Move the cursor
 */
static void move_cursor()
{
	uint16_t pos = (cur_y * 80) + cur_x;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}

/*
 * Scroll one line if necessary
 */
static void scroll()
{
    if(cur_y >= 25)
    {
        int i;
        for (i = 0*80; i < 24*80; i++)
            vmem[i] = vmem[i + 80];

        for (i = 24*80; i < 25*80; i++)
            vmem[i] = MEM_CHAR(' ');

        cur_y = 24;
    }
}

/*
 * Initialize video
 */
void arch_video_init()
{
	arch_video_clear();
	video = VFS_NODE_INITIALIZER(FS_CHARDEVICE);
	strcpy(video.name, "video");
	video.write = NULL;
}

void arch_video_put(char c)
{
	switch (c)
	{
		case '\b' :
			if (cur_x)
			{
				cur_x--;
				vmem[(cur_y * 80) + cur_x] = MEM_CHAR(' ');
			}
			break;

		case '\t' :
			cur_x = (cur_x + 8) & ~(8-1);
			break;

		case '\r' :
			cur_x = 0;
			break;

		case '\n' :
			cur_y++;
			cur_x = 0;
			break;
	
		default:
			vmem[(cur_y * 80) + cur_x] = MEM_CHAR(c);
			cur_x++;
	}

	if (cur_x >= 80)
	{
		cur_x = 0;
		cur_y++;
	}

	scroll();
	move_cursor();
}

void arch_video_clear(void)
{
	int i;
	for (i = 0; i < 80 * 25; i++)
		vmem[i] = MEM_CHAR(' ');

	cur_x = 0;
	cur_y = 0;
	move_cursor();
}

/*
 * Put a null-terminated string on display.
 */
void arch_video_put_s(const char *c)
{
	while (*c)
		arch_video_put(*c++);
}


