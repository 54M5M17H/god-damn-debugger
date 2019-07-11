#!/bin/bash

# build test file
cd ./test
gcc -g -o target.o target.c adder.c
gcc -o ../test/dwarf_test.o ../test/dwarf_test.c ../lib/libdwarf.a
echo "Tests built"