#!/bin/sh

ECHO="echo -e"
DEBUGFS=$(which debugfs)
E2FSCK=$(which e2fsck)

if [ "$DEBUGFS" == "" ]; then

	if [ -x /sbin/debugfs ]; then
		DEBUGFS=/sbin/debugfs
	elif [ -x /usr/sbin/debugfs ]; then
		DEBUGFS=/usr/sbin/debugfs
	elif [ -x /opt/local/sbin/debugfs ]; then
		DEBUGFS=/opt/local/sbin/debugfs
	else
		echo -e "Error: could not find debugfs!"
		echo -e "\tPlease install the e2fsprogs package and make"
		echo -e "\tsure that debugfs is accessible from your PATH."
		exit -1
	fi
fi

if [ "$E2FSCK" == "" ]; then
	if [ -x /sbin/e2fsck ]; then
		E2FSCK=/sbin/e2fsck
	elif [ -x /usr/sbin/e2fsck ]; then
		E2FSCK=/usr/sbin/e2fsck
	elif [ -x /opt/local/e2fsck ]; then
		E2FSCK=/opt/local/sbin/e2fsck
	else
		echo -e "Error: could not find e2fsck!"
		echo -e "\tPlease install the e2fsprogs package and make"
		echo -e "\tsure that e2fsck is accessible from your PATH."
	fi
fi

if [ "$(echo -e)" == "-e" ]; then
	ECHO=echo
fi

cd $(dirname $0)
#cp ../../files/hydrax-fs.img ./hydrax-fs.img
tar -xvf ../../../files/hydrax-fs.xz
${ECHO} 	"cd boot\n" \
		"write ../../../kernel/hydrax hydrax\n" \
		"write ../../../kernel/initrd.img initrd\n" \
		"cd grub\n" \
		"write ../../../files/grub.cfg grub.cfg\n" \
		"cd ../../usr/share/grub\n" \
		"write ../../../files/dejavu_12.pf2 dejavu_12.pf2\n" \
		"write ../../../files/dejavu_16.pf2 dejavu_16.pf2" | \
		${DEBUGFS} -w hydrax-fs.img > /dev/null 2> /dev/null
${E2FSCK} -f hydrax-fs.img > /dev/null 2> /dev/null 
dd if=../../../files/hydrax-pt.img of=../../../hydrax.img > /dev/null 2> /dev/null
dd if=hydrax-fs.img of=../../../hydrax.img seek=2048 > /dev/null 2> /dev/null
rm -f hydrax-fs.img

