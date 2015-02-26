#!/bin/sh

cd $(dirname $0)
#cp ../../files/hydrax-fs.img ./hydrax-fs.img
tar -xvf ../../../files/hydrax-fs.xz
/bin/echo -e 	"cd boot\n" \
		"write ../../../kernel/hydrax hydrax\n" \
		"write ../../../kernel/initrd.img initrd\n" \
		"cd grub\n" \
		"write ../../../files/grub.cfg grub.cfg\n" \
		"cd ../../usr/share/grub\n" \
		"write ../../../files/dejavu_12.pf2 dejavu_12.pf2\n" \
		"write ../../../files/dejavu_16.pf2 dejavu_16.pf2" | \
		/sbin/debugfs -w hydrax-fs.img > /dev/null 2> /dev/null
/sbin/e2fsck -f hydrax-fs.img > /dev/null 2> /dev/null 
dd if=../../../files/hydrax-pt.img of=../../../hydrax.img > /dev/null 2> /dev/null
dd if=hydrax-fs.img of=../../../hydrax.img seek=2048 > /dev/null 2> /dev/null
rm -f hydrax-fs.img

