/*
 * cmos.h
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

#ifndef __CMOS_H__
#define __CMOS_H__

/*
 * read a byte from cmos
 */
unsigned char cmos_readb(unsigned char offset);

/*
 * read from cmos
 *
 */
void cmos_read(unsigned char *buf);

/*
 * write to cmos
 */
void cmos_write(unsigned char *buf);

#endif
