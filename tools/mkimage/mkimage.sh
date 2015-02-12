#!/bin/sh

cd $(dirname $0)
cp ../../files/hydrax-fs.img ./hydrax-fs.img
/sbin/debugfs -w hydrax-fs.img -R "rm hydrax"
/sbin/debugfs -w hydrax-fs.img -R "rm initrd"
/sbin/debugfs -w hydrax-fs.img -R "write ../../kernel/hydrax hydrax"
/sbin/debugfs -w hydrax-fs.img -R "write ../../kernel/initrd.img initrd"
/sbin/e2fsck -f hydrax-fs.img 
dd if=../../files/hydrax-pt.img of=../../hydrax.img
dd if=hydrax-fs.img of=../../hydrax.img seek=2048
rm hydrax-fs.img

