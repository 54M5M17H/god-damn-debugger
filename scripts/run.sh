#!/bin/bash

source $(dirname $0)/change_dir.sh
change_dir

./dist/gdd $(pwd)/test/target.o
