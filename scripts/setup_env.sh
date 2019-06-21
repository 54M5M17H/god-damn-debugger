#!/bin/bash

apt-get update

#install gcc
apt-get -y install gcc
export CXX=gcc

#install cmake
apt-get -y install cmake

