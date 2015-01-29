#!/bin/sh

cd libs
make
cd ../utils
make
cd ../kernel
make
cd ..

