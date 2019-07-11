#!/bin/bash

apt-get update

#install gcc
apt-get -y install gcc
apt-get -y install g++
export CXX=g++

#install cmake
apt-get -y install cmake

# TODO: libdwarf: DOWNLOAD, cmake, make, copy over .a & .h
# TODO: add libdwarf to CMAKE setup
