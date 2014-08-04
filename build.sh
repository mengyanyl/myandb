#!/bin/bash

autoscan
cp configure.in.bak configure.in
aclocal
autoconf
autoheader
automake --add-missing
./configure CXXFLAGS= CFLAGS=
make
