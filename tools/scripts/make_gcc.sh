#!/bin/bash
#
# Some dependencies:
#  automake-1.11
#  aclocal-1.11
#  libtool
#  GNU Make 4.1
#  Autoconf-2.64 (on cygwin: gcc-tools-epoch2-autoconf)
#  GNU Tar, xz, gzip, lzip
#
# On cygwin only i686-pc-mingw32 is
# currently supported/tested.
#  

QUIET=1

progressfilt ()
{
    local flag=false c count cr=$'\r' nl=$'\n'
    while IFS='' read -d '' -rn 1 c
    do
        if $flag
        then
            printf '%c' "$c"
        else
            if [[ $c != $cr && $c != $nl ]]
            then
                count=0
            else
                ((count++))
                if ((count > 1))
                then
                    flag=true
                fi
            fi
        fi
    done
}

dowget()
{
	if [ 1 == 1 ]; then
		wget -qc $1 2>&1 > /dev/null
	else
		wget -c --progress=bar:force -c $1  2>&1 | \
			"$SCRIPTPATH/scripts/make_gcc.sh" --progress-filter
	fi
}

if [ "$1" == "--progress-filter" ]; then
	progressfilt
	exit 0
fi


SCRIPTPATH="$( cd $(dirname $0)/../ ; pwd -P )"
cd "$SCRIPTPATH"
BULLET="\033[0;32m *\033[0m"
REDBUL="\033[0;31m !!\033[0m"
PREFIX="$SCRIPTPATH"
XTARGET=i386-hydrax
MAKE_OPTS=
XBUILD=$(gcc -dumpmachine)
XHOST=$(gcc -dumpmachine)
WIN32=0
JOBS=1

last_error=0

# do our best to detect the host platform
# and default compiler to use
#
case $(uname) in
	CYGWIN*)
		XHOST=i686-pc-mingw32
		JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)
		export PATH=/opt/gcc-tools/epoch2/bin/:$PATH
	;;
	Linux)
		JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)
	;;
esac

# parse arguments
#
while [ $# != 0 ]; do
	case $1 in
		--host=*)
			UXHOST=${1#*=}
			if [ "$UXHOST" != "$XHOST" ]; then
				XHOST=$UXHOST
				PREFIX="$PREFIX/$UXHOST"
			fi

			case ${1#*=} in
				*mingw*)
					WIN32=1
				;;
			esac
		;;
	esac
	shift
done

MAKE_OPTS="-j$JOBS $MAKE_OPTS"

# Print build info
#
echo -e "$BULLET Target: $XTARGET"
echo -e "$BULLET Host: $XHOST"
echo -e "$BULLET Build: $XBUILD"
echo -e "$BULLET Jobs: $JOBS"
echo -e "$BULLET MAKE_OPTS: $MAKE_OPTS"
echo -e "$BULLET Prefix: $PREFIX"

echo -e "$BULLET Copying system includes..."
scripts/copy_headers.sh > /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error copying headers!"
	exit -1
fi

mkdir -p tmp
cd tmp
echo -e "$BULLET Fetching binutils-2.24.tar.gz..."
dowget http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz
echo -e "$BULLET Fetching gcc-4.8.4.tar.gz..."
dowget ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.4/gcc-4.8.4.tar.gz
echo -e "$BULLET Fetching gmp-6.0.0a.tar.lz..."
dowget https://gmplib.org/download/gmp/gmp-6.0.0a.tar.lz
echo -e "$BULLET Fetching mpfr-3.1.2.tar.xz..."
dowget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz
echo -e "$BULLET Fetching mpc-1.0.3.tar.gz..."
dowget ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz

echo -e "$BULLET Cleaning up working directory..."
cd ..
mkdir -p src
cd src
rm -fr binutils-2.24
rm -fr gcc-4.8.4
rm -fr gmp-6.0.0a
rm -fr mpfr-3.1.2
rm -fr mpc-1.0.3
rm -fr build

#if [ "2" == "1" ]; then
echo -e "$BULLET Unpacking binutils-2.24.tar.gz..."
tar -xf ../tmp/binutils-2.24.tar.gz

echo -e "$BULLET Copying config files..."
cp binutils/config.sub binutils-2.24/config.sub
cp binutils/config.bfd binutils-2.24/bfd/config.bfd
cp binutils/gas_configure.tgt binutils-2.24/gas/configure.tgt
cp binutils/ld_configure.tgt binutils-2.24/ld/configure.tgt
cp binutils/elf_i386_hydrax.sh binutils-2.24/ld/emulparams/elf_i386_hydrax.sh
cp binutils/Makefile.am binutils-2.24/ld/Makefile.am
#diff -rupN ../binutils-tmp/binutils-2.24/ binutils-2.24/ > binutils-2.24.patch

#echo -e "$BULLET Applying binutils-2.24.patch..."
#cd binutils-2.24
#patch -p1 < ../patches/binutils-2.24.patch > /dev/null || last_error=1
#if [ "$last_error" == "1" ]; then
#	echo -e "$REDBUL Error applying patches/binutils-2.24.patch!!"
#	exit -1
#fi
#cd ..

echo -e "$BULLET Running automake..."
cd binutils-2.24/ld
aclocal-1.11
automake-1.11
cd ../..

echo -e "\n"
echo -e "$BULLET Configuring binutils..."

mkdir -p build/binutils
cd build/binutils
CMD="
../../binutils-2.24/configure \
	--build=$XBUILD \
	--host=$XHOST \
	--target=$XTARGET \
	--prefix=$PREFIX \
	--with-sysroot=$PREFIX \
	--libdir=$PREFIX \
	--with-lib-path=$PREFIX/lib \
	--disable-nls \
	--with-pkgversion=Hydrax_Binutils \
	--with-bugurl=http://bugs.hydrax.com \
	--disable-werror
"
echo $CMD
echo
${CMD} || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error configuring binutils!"
	exit -1
fi

echo -e "$BULLET Compiling binutils..."
make $MAKE_OPTS || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error compiling binutils!"
	exit -1
fi
echo -e "$BULLET Installing binutils-2.24..."
make install || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error installing binutils!"
	exit -1
fi

echo -e "$BULLET Cleaning up binutils..."
cd ../..
rm -fr build/
rm -fr binutils-2.24/
#fi

echo -e "$BULLET Unpacking gcc-4.8.4.tar.gz..."
tar -xf ../tmp/gcc-4.8.4.tar.gz
echo -e "$BULLET Unpacking gmp-6.0.0a.tar.lz..."
tar -xf ../tmp/gmp-6.0.0a.tar.lz
echo -e "$BULLET Unpacking mpfr-3.1.2.tar.xz..."
tar -xf ../tmp/mpfr-3.1.2.tar.xz
echo -e "$BULLET Unpacking mpc-1.0.3.tar.gz..."
tar -xf ../tmp/mpc-1.0.3.tar.gz

echo -e "$BULLET Copying optional packages..."
mv -f gmp-6.0.0 gcc-4.8.4/gmp
mv -f mpfr-3.1.2 gcc-4.8.4/mpfr
mv -f mpc-1.0.3 gcc-4.8.4/mpc

#echo -e "$BULLET Applying gcc-4.8.4.patch..."
#cd gcc-4.8.4
#patch -p1 < ../patches/gcc-4.8.4.patch > /dev/null || last_error=1
#if [ "$last_error" == "1" ]; then
#	echo -e "$REDBUL Error applying patches/gcc-4.8.4.patch!!"
#	exit -1
#fi
#cd ..

echo -e "Copying configuration files..."
cp -v gcc/config.sub gcc-4.8.4/config.sub
cp -v gcc/hydrax.h gcc-4.8.4/gcc/config/hydrax.h
cp -v gcc/config.gcc gcc-4.8.4/gcc/config.gcc
cp -v gcc/crossconfig.m4 gcc-4.8.4/libstdc++-v3/crossconfig.m4
cp -v gcc/config.host gcc-4.8.4/libgcc/config.host
cp -v gcc/mkfixinc.sh gcc-4.8.4/fixincludes/mkfixinc.sh
#exit -1

echo '#undef STANDARD_STARTFILE_PREFIX' >> gcc-4.8.4/gcc/config/hydrax.h
echo '#define STANDARD_STARTFILE_PREFIX "$PREFIX/i386-hydrax/lib/"' >> gcc-4.8.4/gcc/config/hydrax.h


echo -e "$BULLET Running autoconf on gcc-4.8.4/libstdc++-v3..."
cd gcc-4.8.4/libstdc++-v3
autoconf-2.64 || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error: autoconf-2.64 needed!!"
	exit -1
fi
cd ../..

echo -e "$BULLET Configuring gcc..."
mkdir -p build/gcc
cd build/gcc
../../gcc-4.8.4/configure \
	--target=$XTARGET \
	--prefix=$PREFIX \
	--host=$XHOST \
	--build=$XBUILD \
	--with-sysroot=$PREFIX \
	--disable-multilib \
	--disable-nls \
	--disable-wchar_t \
	--with-pkgversion=Hydrax_GCC \
	--with-bugurl=http://bugs.hydrax.com \
	--enable-languages=c || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error in configure gcc!"
	exit -1
fi
echo -e "$BULLET Building gcc..."
make $MAKE_OPTS all-gcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error in make all-gcc!"
	exit -1
fi
make $MAKE_OPTS all-target-libgcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error in make all-target-libgcc!"
	exit -1
fi
make install-gcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error in make install-gcc!"
	exit -1
fi
make install-target-libgcc || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "$REDBUL Error in make install-target-libgcc!"
	exit -1
fi

cd ../..
rm -fr build/
rm -fr gcc-4.8.4/

cd $SCRIPTPATH
cd bin
if [ $WIN32 == 1 ]; then
	[ -x i386-hydrax-cc.exe ] && rm i386-hydrax-cc.exe
	cp i386-hydrax-gcc.exe i386-hydrax-cc.exe
else
	[ -x i386-hydrax-cc ] && rm i386-hydrax-cc
	ln -s i386-hydrax-gcc i386-hydrax-cc 
fi

