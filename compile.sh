#!/bin/sh
mkdir builds
rm -R builds/linux
mkdir builds/linux
cp config.m4 builds/linux/config.m4
cp src/* builds/linux
cd builds/linux
phpize
./configure --enable-phpcomport
make
cd ../..
