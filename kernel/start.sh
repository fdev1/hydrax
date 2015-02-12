#!/bin/sh

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
cd $SCRIPTPATH

/usr/bin/make
/usr/bin/qemu-system-i386 -S -s -m 256M -hda "$SCRIPTPATH/../hydrax.img" 2> /dev/null &
EMU=$!

/usr/bin/gdb $@
/bin/kill -9 $EMU



#exec kdbg -r localhost:1234 $SCRIPTPATH/hydrax

