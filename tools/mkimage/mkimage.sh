#!/bin/sh

cd $(dirname $0)
cp ../../files/hydrax-fs.img ./hydrax-fs.img
/bin/echo -e 	"cd boot\n" \
		"write ../../kernel/hydrax hydrax\n" \
		"write ../../kernel/initrd.img initrd" | /sbin/debugfs -w hydrax-fs.img
/sbin/e2fsck -f hydrax-fs.img 
dd if=../../files/hydrax-pt.img of=../../hydrax.img
dd if=hydrax-fs.img of=../../hydrax.img seek=2048
rm hydrax-fs.img

