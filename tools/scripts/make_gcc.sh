#!/bin/bash
#
# Some dependencies:
#  autotools
#  libtool
#  GNU Make 4.1
#  GNU Tar, xz, gzip, lzip
#
# On cygwin only i686-pc-mingw32 is
# currently supported/tested.
#  

QUIET=0

progressfilt ()
{
	local flag=false
	local gotnewline=false
	local c=
	local cr=$'\r'
	local nl=$'\n'
	local blank=
	local count=0
	
	while IFS='' read -d '' -rn 1 c
	do
		if [ $flag == true ];
		then
			if [[ $c == $nl ]];
			then
				gotnewline=true
			fi
			if [ $gotnewline == false ];
			then
				printf '%c' "$c"
			fi
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

	COLUMNS=$(tput cols)
	count=0
        while [ $count -lt $COLUMNS ];
        do
                blank="$blank "
                ((count++))
        done

	printf '%c' "$cr"
	printf "$blank"
	printf '%c' "$cr"
}

dowget()
{
	#if [ $QUIET == 1 ]; then
	#	wget -qc $1 2>&1 > /dev/null
	#else
	#fi
	echo -e "$BULLET Fetching $1..."
	wget -c --progress=bar:force -c $1  2>&1 | \
		"$SCRIPTPATH/scripts/make_gcc.sh" --progress-filter
}

dotar()
{
	echo -e "$BULLET Unpacking $(basename $1)..."
	tar -xf $1
}


if [ "$1" == "--progress-filter" ]; then
	progressfilt
	exit 0
fi


SCRIPTPATH="$( cd $(dirname $0)/../ ; pwd -P )"
cd "$SCRIPTPATH"

mkdir -p tmp/tools
AUTOPATH=$(readlink -f tmp/tools)
export PATH=$AUTOPATH/bin:$PATH

BULLET="\033[0;32m *\033[0m"
REDBUL="\033[0;31m !!\033[0m"
PREFIX="$SCRIPTPATH"
XTARGET=i386-hydrax
MAKE_OPTS=
XBUILD=$(gcc -dumpmachine)
XHOST=$(gcc -dumpmachine)
WIN32=0
JOBS=1
BUILD_BINUTILS=1
NEED_AUTOMAKE_1_11=0 

last_error=0 

#if [ "$(which automake-1.11)" == "" ]; then
#	NEED_AUTOMAKE_1_11=1
#fi
if [ ! -x $AUTOPATH/bin/automake-1.11 ] ||
	[ ! -x $AUTOPATH/bin/aclocal-1.11 ] ||
	[ ! -x $AUTOPATH/bin/autoconf-2.64 ]; then
	NEED_AUTOMAKE_1_11=1
fi

# do our best to detect the host platform
# and default compiler to use
#
case $(uname) in
	Darwin)
		JOBS=$(/usr/sbin/sysctl -n hw.ncpu)
		#if [ "$(which automake-1.11)" == "" ]; then
		#	NEED_AUTOMAKE_1_11=1
		#fi
	;;
	CYGWIN*)
		#XHOST=i686-pc-mingw32
		JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)
		#export PATH=/opt/gcc-tools/epoch2/bin/:$PATH
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


export PATH=$PREFIX:$PATH

echo -e "$BULLET Copying system includes..."
scripts/copy_headers.sh > /dev/null || last_error=1
if [ "$last_error" == "1" ]; then
	echo -e "Error copying headers!"
	exit -1
fi

mkdir -p tmp
cd tmp
dowget http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz
dowget ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.4/gcc-4.8.4.tar.gz
dowget https://gmplib.org/download/gmp/gmp-6.0.0a.tar.lz
dowget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz
dowget ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
#dowget http://isl.gforge.inria.fr/isl-0.14.tar.gz
#dowget http://www.bastoul.net/cloog/pages/download/cloog-0.18.3.tar.gz

if [ $NEED_AUTOMAKE_1_11 == 1 ]; then
	dowget http://ftp.gnu.org/gnu/automake/automake-1.11.1.tar.gz
	dowget http://ftp.gnu.org/gnu/autoconf/autoconf-2.64.tar.xz
	dowget http://gnu.mirrors.pair.com/gnu/libtool/libtool-2.4.tar.xz
fi

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

if [ $BUILD_BINUTILS == 1 ]; then

	if [ $NEED_AUTOMAKE_1_11 == 1 ]; then

		dotar ../tmp/automake-1.11.1.tar.gz
		dotar ../tmp/autoconf-2.64.tar.xz
		dotar ../tmp/libtool-2.4.tar.xz
		
		cd autoconf-2.64
		echo -e "$BULLET Configuring autoconf-2.64..."
		./configure --prefix=$AUTOPATH || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error configuring autoconf-2.64"
			exit -1
		fi
		echo -e "$BULLET Compiling autoconf-2.64..."
		make $MAKE_OPTS || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error compiling autoconf-2.64!!"
			exit -1
		fi
		echo -e "$BULLET Installing autoconf-2.64..."
		make install || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error installing autoconf!"
			exit -1
		fi
		echo -e "$BULLET Cleaning up autoconf-2.64..."
		cd ..
		rm -fR autoconf-2.64

		cd automake-1.11.1
		echo -e "$BULLET Configuring automake-1.11.6..."
		./configure --prefix=$AUTOPATH || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error configuring automake-1.11!!"
			exit -1
		fi
		echo -e "$BULLET Compiling automake-1.11..."
		make $MAKE_OPTS || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error compiling automake-1.11!!"
			exit -1
		fi
		echo -e "$BULLET Installing automake-1.11..."
		make install || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error installing automake-1.11!!"
			exit -1
		fi
		echo -e "$BULLET Cleaning up automake-1.11..."
		cd ..
		rm -fR automake-1.11.1

		echo -e "$BULLET Configuring libtool-2.4..."
		cd libtool-2.4
		./configure --prefix=$AUTOPATH || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error configuring libtool..."
			exit -1
		fi
		echo -e "$BULLET Compiling libtool..."
		make $MAKE_OPTS || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error compiling libtool..."
			exit -1
		fi
		echo -e "$BULLET Installing libtool..."
		make install || last_error=1
		if [ $last_error == 1 ]; then
			echo -e "$REDBUL Error installing libtool..."
			exit -1
		fi
		ln -s $AUTOPATH/bin/autoconf $AUTOPATH/bin/autoconf-2.64
		echo -e "$BULLET Cleaning libtool..."
		cd ..
		rm -fR libtool-2.4

	fi

	dotar ../tmp/binutils-2.24.tar.gz
	#dotar ../tmp/isl-0.14.tar.gz
	#dotar ../tmp/cloog-0.18.3.tar.gz
	#dotar ../tmp/gmp-6.0.0a.tar.lz
	#echo -e "$BULLET Preparing source..."
	#mv isl-0.14 binutils-2.24/isl
	#mv cloog-0.18.3 binutils-2.24/cloog
	#mv gmp-6.0.0 binutils-2.24/gmp

	echo -e "$BULLET Customizing source..."
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
		--disable-nls \
		--with-pkgversion=Hydrax_Binutils \
		--with-bugurl=http://bugs.hydrax.com \
		--disable-werror
	"
	[ $QUIET == 1 ] || echo $CMD 
	[ $QUIET == 1 ] || echo
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

fi

dotar ../tmp/gcc-4.8.4.tar.gz
dotar ../tmp/gmp-6.0.0a.tar.lz
dotar ../tmp/mpfr-3.1.2.tar.xz
dotar ../tmp/mpc-1.0.3.tar.gz
#dotar ../tmp/isl-0.14.tar.gz
#dotar ../tmp/cloog-0.18.3.tar.gz

echo -e "$BULLET Copying optional packages..."
mv -f gmp-6.0.0 gcc-4.8.4/gmp
mv -f mpfr-3.1.2 gcc-4.8.4/mpfr
mv -f mpc-1.0.3 gcc-4.8.4/mpc
#mv -f isl-0.14 gcc-4.8.4/isl
#mv -f cloog-0.18.3 gcc-4.8.4/cloog

#echo -e "$BULLET Applying gcc-4.8.4.patch..."
#cd gcc-4.8.4
#patch -p1 < ../patches/gcc-4.8.4.patch > /dev/null || last_error=1
#if [ "$last_error" == "1" ]; then
#	echo -e "$REDBUL Error applying patches/gcc-4.8.4.patch!!"
#	exit -1
#fi
#cd ..

echo -e "$BULLET Customizing source..."
cp gcc/config.sub gcc-4.8.4/config.sub
cp gcc/hydrax.h gcc-4.8.4/gcc/config/hydrax.h
cp gcc/config.gcc gcc-4.8.4/gcc/config.gcc
cp gcc/crossconfig.m4 gcc-4.8.4/libstdc++-v3/crossconfig.m4
cp gcc/config.host gcc-4.8.4/libgcc/config.host
cp gcc/mkfixinc.sh gcc-4.8.4/fixincludes/mkfixinc.sh

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
	--disable-multilib \
	--disable-nls \
	--disable-wchar_t \
	--with-pkgversion=Hydrax_GCC \
	--with-bugurl=http://bugs.hydrax.com \
	--enable-languages=c || last_error=1
	#--with-sysroot=$PREFIX \
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

