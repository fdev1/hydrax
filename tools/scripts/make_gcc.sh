#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH


PREFIX="$SCRIPTPATH"
TARGET=i386-hydrax
#PATH="$PREFIX/bin:$PATH"
MAKE_OPTS=-j2

last_error=0

echo -e "Copying system includes..."
scripts/copy_headers.sh > /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error copying headers!"
	exit -1
fi

mkdir -p tmp
cd tmp
echo -e "Fetching binutils-2.24.tar.gz..."
wget -c http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz 2> /dev/null
echo -e "Fetching gcc-4.8.4.tar.gz..."
wget -c ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.4/gcc-4.8.4.tar.gz 2> /dev/null
echo -e "Fetching gmp-6.0.0a.tar.lz..."
wget -c https://gmplib.org/download/gmp/gmp-6.0.0a.tar.lz 2> /dev/null
echo -e "Fetching mpfr-3.1.2.tar.xz..."
wget -c http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz 2> /dev/null
echo -e "Fetching mpc-1.0.3.tar.gz..."
wget -c ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz 2> /dev/null

echo -e "Cleaning up working directory..."
cd ..
mkdir -p src
cd src
rm -fr binutils-2.24
rm -fr gcc-4.8.1
rm -fr gmp-6.0.0a
rm -fr mpfr-3.1.2
rm -fr mpc-1.0.3
rm -fr build-binutils
rm -fr build-gcc

echo -e "Unpacking binutils-2.24.tar.gz..."
tar -xf ../tmp/binutils-2.24.tar.gz

echo -e "Copying config files..."
cp binutils/config.sub binutils-2.24/config.sub
cp binutils/config.bfd binutils-2.24/bfd/config.bfd
cp binutils/gas_configure.tgt binutils-2.24/gas/configure.tgt
cp binutils/ld_configure.tgt binutils-2.24/ld/configure.tgt
cp binutils/elf_i386_hydrax.sh binutils-2.24/ld/emulparams/elf_i386_hydrax.sh
cp binutils/Makefile.am binutils-2.24/ld/Makefile.am

echo -e "Running automake..."
cd binutils-2.24/ld
aclocal-1.13
automake-1.13
cd ../..

if [ "1" == "1" ]; then
echo -e "\n"
echo -e "Configuring binutils..."
mkdir -p build/binutils
cd build/binutils
../../binutils-2.24/configure --target=$TARGET --prefix="$PREFIX" \
	--with-sysroot="$PREFIX" --disable-nls --disable-werror || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error configuring binutils!"
	exit -1
fi

echo -e "Compiling binutils..."
make $MAKE_OPTS || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error compiling binutils!"
	exit -1
fi
make install || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error installing binutils!"
	exit -1
fi

echo -e "Cleaning up binutils..."
cd ../..
rm -fr build/
rm -fr binutils-2.24/
fi

echo -e "Unpacking gcc-4.8.4.tar.gz..."
tar -xf ../tmp/gcc-4.8.4.tar.gz
echo -e "Unpacking gmp-6.0.0a.tar.lz..."
tar -xf ../tmp/gmp-6.0.0a.tar.lz
echo -e "Unpacking mpfr-3.1.2.tar.xz..."
tar -xf ../tmp/mpfr-3.1.2.tar.xz
echo -e "Unpacking mpc-1.0.3.tar.gz..."
tar -xf ../tmp/mpc-1.0.3.tar.gz

echo -e "Copying optional packages..."
mv -f gmp-6.0.0 gcc-4.8.4/gmp
mv -f mpfr-3.1.2 gcc-4.8.4/mpfr
mv -f mpc-1.0.3 gcc-4.8.4/mpc

echo -e "Copying configuration files..."
cp -v gcc/config.sub gcc-4.8.4/config.sub
cp -v gcc/hydrax.h gcc-4.8.4/gcc/config/hydrax.h
cp -v gcc/config.gcc gcc-4.8.4/gcc/config.gcc
cp -v gcc/crossconfig.m4 gcc-4.8.4/libstdc++-v3/crossconfig.m4
cp -v gcc/config.host gcc-4.8.4/libgcc/config.host
cp -v gcc/mkfixinc.sh gcc-4.8.4/fixincludes/mkfixinc.sh

echo -e "Running autoconf on gcc-4.8.4/libstdc++-v3..."
cd gcc-4.8.4/libstdc++-v3
autoconf-2.64
cd ../..

echo -e "Configuring gcc..."
mkdir -p build/gcc
cd build/gcc
../../gcc-4.8.4/configure --target=$TARGET --prefix="$PREFIX" \
	--with-sysroot="$PREFIX" --disable-multilib --disable-nls --enable-languages=c || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error in configure gcc!"
	exit -1
fi
echo -e "Building gcc..."
make $MAKE_OPTS all-gcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error in make all-gcc!"
	exit -1
fi
make $MAKE_OPTS all-target-libgcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error in make all-target-gcc!"
	exit -1
fi
make install-gcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error in make install-gcc!"
	exit -1
fi
make install-target-libgcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error in make-install-target-libgcc!"
	exit -1
fi

cd ../..
rm -fr build/
rm -fr gcc-4.8.4/


