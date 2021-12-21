#!/bin/sh

mkdir tmp
cd tmp

VERSION=3.0.2

#############################################
## download libressl-$VERSION

wget https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-$VERSION.tar.gz

#############################################
## clean dir

rm -rf libressl-$VERSION

#############################################
## unpack

tar -xvzf libressl-$VERSION.tar.gz
cd libressl-$VERSION

#############################################
## build and install libressl

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
