#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH

BULLET="\033[0;32m *\033[0m"
REDBUL="\033[0;31m !!\033[0m"
PREFIX="$SCRIPTPATH"
TARGET=i386-hydrax
MAKE_OPTS=-j2
PATH="$SCRIPTPATH/bin:$PATH"

last_error=0

mkdir -p tmp
cd tmp
echo -e "$BULLET Fetching ftp://sourceware.org/pub/newlib/newlib-2.2.0.20150225.tar.gz..."
wget -c --quiet --show-progress ftp://sourceware.org/pub/newlib/newlib-2.2.0.20150225.tar.gz
cd ..

cd src
echo -e "$BULLET Cleaning build directory..."
rm -fr newlib-2.2.0.20150225/
rm -fr build/

echo -e "$BULLET Unpacking newlib-2.2.0.20150225.tar.gz..."
tar -xf ../tmp/newlib-2.2.0.20150225.tar.gz

#echo -e "$BULLET copying config files..."
#mkdir -p newlib-2.2.0.20150225/newlib/libc/sys/hydrax/
#cp -v newlib/config.sub newlib-2.2.0.20150225/config.sub
#cp -v newlib/configure.host newlib-2.2.0.20150225/newlib/configure.host
#cp -v newlib/configure.in newlib-2.2.0.20150225/newlib/libc/sys/configure.in
#cp -v newlib/crt0.S newlib-2.2.0.20150225/newlib/libc/sys/hydrax/crt0.S
#cp -v newlib/hydrax_configure.in newlib-2.2.0.20150225/newlib/libc/sys/hydrax/configure.in
#cp -v newlib/hydrax_Makefile.am newlib-2.2.0.20150225/newlib/libc/sys/hydrax/Makefile.am
#cp -v newlib/syscalls.c newlib-2.2.0.20150225/newlib/libc/sys/hydrax/syscalls.c
#cp -v newlib/config.h newlib-2.2.0.20150225/newlib/libc/include/sys/config.h
#exit -1

echo -e "$BULLET Applying newlib-2.2.0.20150225.patch..."
cd newlib-2.2.0.20150225
patch -p1 < ../patches/newlib-2.2.0.20150225.patch || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Could not apply patch!!"
	exit -1
fi
cd ../

echo -e "$BULLET Running autoconf..."
cd newlib-2.2.0.20150225/newlib/libc/sys
autoconf || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Autoconf failed on newlib/libc/sys!!"
	exit -1
fi
cd hydrax
aclocal-1.11 -I../../.. -I../../../.. || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Aclocal failed on newlib/libc/sys/hydrax"
	exit -1
fi
autoconf-2.64 || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Autoreconf failed on newlib/libc/sys/hydrax!!"
	exit -1
fi
automake-1.11 --add-missing || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Automake failed!!"
	exit -1
fi
cd ../../../../..

echo -e "$BULLET Configuring newlib..."
mkdir -p build/newlib
cd build/newlib
../../newlib-2.2.0.20150225/configure --prefix="$PREFIX" \
	--with-sysroot=$PREFIX --target=i386-hydrax || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error configuring newlib!!"
	exit -1
fi
make all || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error compiling newlib!!"
	exit -1
fi
echo -e "$BULLET Installing newlib to '$PREFIX'..."
make install || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error install newlib!!"
	exit -1
fi
cd ../..

echo -e "$BULLET newlib compiled successfuly."
rm -fr newlib-2.2.0.20150225/
rm -fr build/
exit 0



