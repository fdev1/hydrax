#include <arch/arch.h>
#include <vfs.h>

/*
 * Gets the system time.
 */
time_t time(time_t *t)
{
	time_t t2;
	vfs_node_t *rtc;
	rtc = vfs_open("/dev/rtc", NULL);
	if (unlikely(rtc == NULL))
	{
		*t = 0;
		return *t;
	}
	
	vfs_read(rtc, 0, sizeof(time_t), (unsigned char*) &t2);
	*t = t2;
	return t2;
}
