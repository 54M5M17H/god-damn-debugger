#!/bin/bash

# build test file
cd ./test
gcc -g -static -o target.o target.c adder.c
gcc -o ./dwarf_test.o ../test/dwarf_test.c ../lib/libdwarf.a
echo "Tests built"
