#ifndef __UTSNAME_H__
#define __UTSNAME_H__

#define SYSNAME_MAX		(255)
#define SYSVER_MAX		(32)

struct utsname
{
	char sysname[SYSNAME_MAX];
	char nodename[1];
	char release[SYSVER_MAX];
	char version[SYSVER_MAX];
	char machine[SYSVER_MAX];
};

/*
 * Get name and info about current kernel.
 */
int uname(struct utsname *buf);

#endif
