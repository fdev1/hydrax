#!/bin/bash
SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH

lasterr=0
scripts/make_gcc.sh || lasterr=1
if [ "$lasterr" == "1" ]; then
	exit -1
fi
scripts/make_newlib.sh || lasterr=1
if [ "$lasterr" == "1" ]; then
	exit -1
fi

