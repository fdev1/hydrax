#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH

BULLET="\033[0;32m *\033[0m"
REDBUL="\033[0;31m !!\033[0m"
PREFIX="$SCRIPTPATH"
TARGET=i386-hydrax
MAKE_OPTS=-j2
PATH="$SCRIPTPATH/bin:$PATH"
NEWLIB_SRC=newlib-2.2.0.20150225
last_error=0

echo -e "$BULLET Copying system includes..."
scripts/copy_headers.sh > /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
        echo -e "Error copying headers!"
        exit -1
fi

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

echo -e "$BULLET copying config files..."
mkdir -p newlib-2.2.0.20150225/newlib/libc/sys/hydrax/
cp newlib/config.sub newlib-2.2.0.20150225/config.sub
cp newlib/configure.host newlib-2.2.0.20150225/newlib/configure.host
cp newlib/configure.in newlib-2.2.0.20150225/newlib/libc/sys/configure.in
cp newlib/crt0.c newlib-2.2.0.20150225/newlib/libc/sys/hydrax/crt0.c
cp newlib/hydrax_configure.in newlib-2.2.0.20150225/newlib/libc/sys/hydrax/configure.in
cp newlib/hydrax_Makefile.am newlib-2.2.0.20150225/newlib/libc/sys/hydrax/Makefile.am

cp newlib/syscalls.h $NEWLIB_SRC/newlib/libc/sys/hydrax/syscalls.h
cp newlib/exit.c $NEWLIB_SRC/newlib/libc/sys/hydrax/exit.c
cp newlib/open.c $NEWLIB_SRC/newlib/libc/sys/hydrax/open.c
cp newlib/close.c $NEWLIB_SRC/newlib/libc/sys/hydrax/close.c
cp newlib/fork.c $NEWLIB_SRC/newlib/libc/sys/hydrax/fork.c
cp newlib/execve.c $NEWLIB_SRC/newlib/libc/sys/hydrax/execve.c
cp newlib/pipe.c $NEWLIB_SRC/newlib/libc/sys/hydrax/pipe.c
cp newlib/getpid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/getpid.c
cp newlib/kill.c $NEWLIB_SRC/newlib/libc/sys/hydrax/kill.c
cp newlib/read.c $NEWLIB_SRC/newlib/libc/sys/hydrax/read.c
cp newlib/write.c $NEWLIB_SRC/newlib/libc/sys/hydrax/write.c
cp newlib/stat.c $NEWLIB_SRC/newlib/libc/sys/hydrax/stat.c
cp newlib/signal.c $NEWLIB_SRC/newlib/libc/sys/hydrax/signal.c
cp newlib/uname.c $NEWLIB_SRC/newlib/libc/sys/hydrax/uname.c
cp newlib/getuid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/getuid.c
cp newlib/getgid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/getgid.c
cp newlib/setuid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/setuid.c
cp newlib/setgid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/setgid.c
cp newlib/getcwd.c $NEWLIB_SRC/newlib/libc/sys/hydrax/getcwd.c
cp newlib/chdir.c $NEWLIB_SRC/newlib/libc/sys/hydrax/chdir.c
cp newlib/waitpid.c $NEWLIB_SRC/newlib/libc/sys/hydrax/waitpid.c
cp newlib/dup.c $NEWLIB_SRC/newlib/libc/sys/hydrax/dup.c
cp newlib/dup2.c $NEWLIB_SRC/newlib/libc/sys/hydrax/dup2.c
cp newlib/readdir.c $NEWLIB_SRC/newlib/libc/sys/hydrax/readdir.c

cp newlib/syscalls.c newlib-2.2.0.20150225/newlib/libc/sys/hydrax/syscalls.c
cp newlib/config.h newlib-2.2.0.20150225/newlib/libc/include/sys/config.h

echo -e "$BULLET Applying newlib-2.2.0.20150225.patch..."
cd newlib-2.2.0.20150225
#patch -p1 < ../patches/newlib-2.2.0.20150225.patch || last_error=1
#if [ "$last_error" == "1" ]; then
#	echo -e "$REDBUL Could not apply patch!!"
#	exit -1
#fi

# Copy required system headers to hydrax directory
#
mkdir -p newlib/libc/sys/hydrax/include/sys
cp ../../../kernel/include/dirent.h newlib/libc/sys/hydrax/include/sys/dirent.h
cp ../../../kernel/include/signal.h newlib/libc/sys/hydrax/include/sys/signal.h
cp ../../../kernel/include/pthread.h newlib/libc/sys/hydrax/include/sys/pthread.h
cp ../../../kernel/include/unistd.h newlib/libc/sys/hydrax/include/sys/unistd.h
cp ../../../kernel/include/sys/stat.h newlib/libc/sys/hydrax/include/sys/stat.h
cp ../../../kernel/include/sys/types.h newlib/libc/sys/hydrax/include/sys/types.h
cp ../../../kernel/include/errno.h newlib/libc/sys/hydrax/include/sys/errno.h
cd ../

echo -e "$BULLET Running autoconf..."
cd newlib-2.2.0.20150225/newlib/libc/sys
autoconf > /dev/null 2> /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Autoconf failed on newlib/libc/sys!!"
	exit -1
fi
cd hydrax
aclocal-1.11 -I../../.. -I../../../.. > /dev/null 2> /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Aclocal failed on newlib/libc/sys/hydrax"
	exit -1
fi
autoconf-2.64 > /dev/null 2> /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Autoreconf failed on newlib/libc/sys/hydrax!!"
	exit -1
fi
automake-1.11 --add-missing > /dev/null 2> /dev/null || last_error=1
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
make DESTDIR=${SYSROOT} install || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error install newlib!!"
	exit -1
fi
cd ../..

echo -e "$BULLET newlib compiled successfuly."
rm -fr newlib-2.2.0.20150225/
rm -fr build/
exit 0



