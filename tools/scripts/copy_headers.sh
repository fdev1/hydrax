#!/bin/bash
SCRIPTPATH=$( cd $(dirname $0)/.. ; pwd -P )
cd $SCRIPTPATH

mkdir -p usr/include
mkdir -p include
cp -vR ../libs/*.h usr/include/
cp -vRT ../kernel/include usr/include

cp -vR ../libs/*.h include/
cp -vRT ../kernel/include include/
