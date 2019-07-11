#!/bin/bash

source ./change_dir.sh
change_dir
cd ./downloads
apt-get install wget

wget https://www.prevanders.net/libdwarf-20190529.tar.gz
DIR_NAME=$(echo https://www.prevanders.net/libdwarf-20190529.tar.gz | sed -E "s/.*\///")
sha256sum $DIR_NAME
tar -zxf $DIR_NAME
cd libdwarf-20190529
./configure
cmake .
make

cp libdwarf/.libs/libdwarf.a ../../libs/
cp libdwarf/dwarf.h ../../libs/
cp libdwarf/libdwarf.h ../../libs/