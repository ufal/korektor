#!/bin/sh

set -e

rm -rf libmicrohttpd.so* microhttpd.h

cd libmicrohttpd-*/
./configure --disable-curl --disable-https
make -j4
cp -a src/microhttpd/.libs/libmicrohttpd.so* src/include/microhttpd.h ..
make distclean
