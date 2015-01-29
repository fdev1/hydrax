/*
 * rtc.c
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

#include <arch/platform.h>
#include <printk.h>
#include <time.h>
#include <vfs.h>
#include <arch/rtc.h>
#include <kheap.h>
#include <string.h>
#include <devfs.h>
#include <arch/cmos.h>

#define YEAR        2014
 
int century_register = 0x00;                                // Set by ACPI table parsing code if possible
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

static const int days[] =
{
	31, 28,	31, 30,	31, 30,	31, 31,	30, 31,	30, 31
};


int get_update_in_progress_flag() 
{
	return (cmos_readb(0xA) & 0x80);
}

/*
 * Initialize the real-time clock
 */
void rtc_init(void)
{
	int r;
	vfs_node_t *rtc;
	rtc = (vfs_node_t*) malloc(sizeof(vfs_node_t)); /* we dont need to track this */
	if (rtc == NULL)
		panic("rtc_init: out of memory!");
	*rtc = vfs_node_init(FS_CHARDEVICE);
	strcpy(rtc->name, "rtc");
	rtc->read = (read_type_t) &rtc_read;
	rtc->write = NULL;

	r = devfs_mknod(rtc);
	if (r == -1)
		panic("rtc_init: could not create dev node!");

	printk(7, "rtc_init: rtc ready");
	return;
}

void rtc_read(int offset, size_t size, time_t *time) 
{
	unsigned char century;
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char last_century;
	unsigned char registerB;
 
	// Note: This uses the "read registers until you get the same values twice in a row" technique
	//       to avoid getting dodgy/inconsistent values due to RTC updates

	while (get_update_in_progress_flag());                // Make sure an update isn't in progress
	second = cmos_readb(0x00);
	minute = cmos_readb(0x02);
	hour = cmos_readb(0x04);
	day = cmos_readb(0x07);
	month = cmos_readb(0x08);
	year = cmos_readb(0x09);

	if (century_register != 0) 
		century = cmos_readb(century_register);
 
	do 
	{
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		last_century = century;
            
		while (get_update_in_progress_flag());           // Make sure an update isn't in progress
		second = cmos_readb(0x00);
		minute = cmos_readb(0x02);
		hour = cmos_readb(0x04);
		day = cmos_readb(0x07);
		month = cmos_readb(0x08);
		year = cmos_readb(0x09);

		if(century_register != 0) 
			century = cmos_readb(century_register);
            
	} 
	while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
               (last_day != day) || (last_month != month) || (last_year != year) ||
               (last_century != century) );
 
      
	registerB = cmos_readb(0x0B);
 
      
      
	// Convert BCD to binary values if necessary
 
      
	if (!(registerB & 0x04)) 
	{
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
            
		if(century_register != 0) 
			century = (century & 0x0F) + ((century / 16) * 10);
	}
 
	// Convert 12 hour clock to 24 hour clock if necessary
 
      
	if (!(registerB & 0x02) && (hour & 0x80)) 
		hour = ((hour & 0x7F) + 12) % 24;
 
      
	// Calculate the full (4-digit) year
 
      
	if(century_register != 0) 
	{
		year += century * 100;
	} 
	else 
	{
		year += (YEAR / 100) * 100;
		if(year < YEAR) year += 100;
	}
	//printk(8, "%i:%i:%i %i/%i/%i", 
	//	hour, minute, second, month, day, year);

	*time = 0;
	*time += (year - 1970) * 365 * 24 * 60 * 60;
	*time += days[month] * 24 * 60 * 60;
	*time += day * 24 * 60 * 60;
	*time += hour * 60 * 60;
	*time += minute * 60;
	*time += second;
}

