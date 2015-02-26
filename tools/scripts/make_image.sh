#!/bin/bash

# make sure we have superuser access
#
if [[ $EUID -ne 0 ]]; then
	echo -e 'Superuser access required.'
	exit -1
fi


BULLET="\033[0;32m *\033[0m"
REDBUL="\033[0;31m !!\033[0m"
IMAGEBASE=$1

if [ "$IMAGEBASE" == "" ]; then
	IMAGEBASE=hydrax
fi

IMAGENAME=$IMAGEBASE.img
IMAGENAME_PT=$IMAGEBASE-pt.img
IMAGENAME_FS=$IMAGEBASE-fs.img
IMAGENAME_XZ=$IMAGEBASE-fs.xz

failed=0

rm -rf $IMAGENAME
rm -rf $IMAGENAME_PT
rm -rf $IMAGENAME_FS
rm -rf $IMAGENAME_XZ
echo -e "$BULLET Allocating image ($IMAGENAME)..."
dd if=/dev/zero of=$IMAGENAME bs=512 count=131072 2> /dev/null || failed=1
if [ "$failed" == "1" ]; then
	echo -e "$REDBUL Error allocating image!!"
	exit -1
fi

echo -e "$BULLET Partitioning image..."
fdisk $IMAGENAME <<EOF > /dev/null 2> /dev/null
n
p
1
2048
131071
w
EOF

echo -e "$BULLET Setting loopback device..."
sudo losetup /dev/loop0 $IMAGENAME || failed=1
if [ "$failed" == "1" ]; then
	echo -e "$REDBUL Could not setup loopback device at /dev/loop0!!"
	exit -1
fi
sudo losetup /dev/loop1 $IMAGENAME -o 1048576 || failed=1
if [ "$failed" == "1" ]; then
	echo -e "$REDBUL Could not setup loopback device at /dev/loop1!!"
	sudo losetup -d /dev/loop0
	exit -1
fi

echo -e "$BULLET Formatting image..."
sudo mke2fs /dev/loop1 2> /dev/null > /dev/null || failed=1
if [ "$failed" == "1" ]; then
	echo -e "$REDBUL Could not format image!!"
	sudo losetup -d /dev/loop0
	sudo losetup -d /dev/loop1
	exit -1
fi

echo -e "$BULLET Mounting volume..."
mkdir -p $(pwd)/mnt
sudo mount /dev/loop1 $(pwd)/mnt || failed=1
if [ "$failed" == "1" ]; then
	echo -e "$REDBUL Could not mount image!!"
	sudo losetup -d /dev/loop0
	sudo losetup -d /dev/loop1
	exit -1
fi

echo -e "$BULLET Installing GRUB2..."
sudo grub2-install --root-directory=$(pwd)/mnt \
	--directory=/usr/lib/grub/i386-pc \
	--no-floppy \
	--modules="normal part_msdos ext2 multiboot" \
	--force /dev/loop0 > /dev/null 2> /dev/null

echo -e "$BULLET Cleaning up..."
sudo umount $(pwd)/mnt
sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1
rm -fr $(pwd)/mnt/

echo -e "$BULLET Splitting image..."
dd if=$IMAGENAME of=$IMAGENAME_PT bs=512 count=2048 > /dev/null 2> /dev/null
dd if=$IMAGENAME of=$IMAGENAME_FS bs=512 skip=2048 > /dev/null 2> /dev/null

echo -e "$BULLET Compressing filesystem image ($IMAGENAME_XZ)..."
tar --xz -cf $IMAGENAME_XZ $IMAGENAME_FS

echo -e "$BULLET Cleaning up..."
rm -f $IMAGENAME
rm -f $IMAGENAME_FS
