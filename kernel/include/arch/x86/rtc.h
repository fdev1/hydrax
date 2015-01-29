/*
 * include/arch/x86/rtc.h
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

#ifndef __RTC_H__
#define __RTC_H__

void rtc_init(void);

void rtc_read(int offset, size_t size, time_t *t);

#endif

