#!/bin/bash

#
# Cywin dependencies:
#
#   gcc-core
#   binutils
#   autoconf-2.5
#   make
#   lzip
#   xz
#   wget
#   
#


SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH

if [ 1 == 2 ]; then
lasterr=0
scripts/make_gcc.sh || lasterr=1
if [ "$lasterr" == "1" ]; then
	exit -1
fi
scripts/make_newlib.sh || lasterr=1
if [ "$lasterr" == "1" ]; then
	exit -1
fi
fi

last_error=0

cd src/symclean
make || last_error=1
if [ $last_error == 1 ]; then
	echo "Error: could not build symclean!!"
	exit -1
fi
cd ../..

cd src/mkinitrd
make || last_error=1
if [ $last_error == 1 ]; then
	echo "Error: could not build mkinitrd!!"
	exit -1
fi

