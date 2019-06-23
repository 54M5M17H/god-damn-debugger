#!/bin/bash

apt-get update

#install gcc
apt-get -y install gcc
apt-get -y install g++
export CXX=g++

#install cmake
apt-get -y install cmake
