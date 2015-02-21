#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
cd $SCRIPTPATH

export PREFIX="$SCRIPTPATH/build"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"


MAKE_OPTS=-j2

mkdir -p tmp
cd tmp
wget -c http://ftp.gnu.org/gnu/binutils/binutils-2.25.tar.gz
wget -c ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.4/gcc-4.8.4.tar.gz
wget -c https://gmplib.org/download/gmp/gmp-6.0.0a.tar.lz
wget -c http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz
wget -c ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz

cd ..
rmdir -f binutils-2.25
rmdir -f gcc-4.8.1
rmdir -f gmp-6.0.0a
rmdir -f mpfr-3.1.2
rmdir -f mpc-1.0.3
rmdir -f build-binutils
rmdir -f build-gcc

tar -xvf tmp/binutils-2.25.tar.gz
tar -xvf tmp/gcc-4.8.4.tar.gz
tar -xvf tmp/gmp-6.0.0a.tar.lz
tar -xvf tmp/mpfr-3.1.2.tar.xz
tar -xvf tmp/mpc-1.0.3.tar.gz

mkdir -p build-binutils
cd build-binutils
../binutils-2.25/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make $MAKE_OPTS
make install

cd ..
mv gmp-6.0.0a gcc-4.8.4/gmp
mv mpfr-3.1.2 gcc-4.8.4/mpfr
mv mpc-1.0.3 gcc-4.8.4/mpc
  
mkdir -p build-gcc
cd build-gcc
../gcc-4.8.4/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make $MAKE_OPTS all-gcc
make $MAKE_OPTS all-target-libgcc
make install-gcc
make install-target-libgcc

