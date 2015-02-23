#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
cd $SCRIPTPATH

PREFIX="$SCRIPTPATH"
TARGET=i386-hydrax
PATH="$PREFIX/bin:$PATH"
MAKE_OPTS=-j2

mkdir -p usr/include
cp -RT ../libs/*.h usr/include
cp -RT ../kernel/include usr/include

echo -e "Downloading required packages..."
mkdir -p tmp
cd tmp
wget -c http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz
wget -c ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.4/gcc-4.8.4.tar.gz
wget -c https://gmplib.org/download/gmp/gmp-6.0.0a.tar.lz
wget -c http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz
wget -c ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz

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

echo -e "\n"
echo -e "Configuring binutils..."
mkdir -p build/binutils
cd build/binutils
../../binutils-2.24/configure --target=$TARGET --prefix="$PREFIX" \
	--with-sysroot="$PREFIX" --disable-nls --disable-werror

echo -e "Compiling binutils..."
make $MAKE_OPTS
make install

echo -e "Cleaning up binutils..."
cd ../..
rm -fr build/
rm -fr binutils-2.24/

echo -e "Unpacking gcc-4.8.4.tar.gz..."
tar -xf ../tmp/gcc-4.8.4.tar.gz
echo -e "Unpacking gmp-6.0.0a.tar.lz..."
tar -xf ../tmp/gmp-6.0.0a.tar.lz
echo -e "Unpacking mpfr-3.1.2.tar.xz..."
tar -xf ../tmp/mpfr-3.1.2.tar.xz
echo -e "Unpacking mpc-1.0.3.tar.gz..."
tar -xf ../tmp/mpc-1.0.3.tar.gz

mv -f gmp-6.0.0 gcc-4.8.4/gmp
mv -f mpfr-3.1.2 gcc-4.8.4/mpfr
mv -f mpc-1.0.3 gcc-4.8.4/mpc

echo -e "Configuring gcc..."
mkdir -p build/gcc
cd build/gcc
../../gcc-4.8.4/configure --target=i386-elf --prefix="$PREFIX" \
	--with-sysroot="$PREFIX" --disable-nls --enable-languages=c --without-headers
echo -e "Building gcc..."
make $MAKE_OPTS all-gcc
make $MAKE_OPTS all-target-libgcc
make install-gcc
make install-target-libgcc

cd ../..
rm -fr build/
rm -fr gcc-4.8.4/


